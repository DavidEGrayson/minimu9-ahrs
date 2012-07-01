/*
promising:
http://elinux.org/Interfacing_with_I2C_Devices
https://i2c.wiki.kernel.org/index.php/Main_Page
*/

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "LSM303.h"
#include "L3G4200D.h"
#include <sys/time.h>
#include <Eigen/Geometry>

int millis()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec) * 1000 + (tv.tv_usec)/1000;
}

void streamRawValues(LSM303& compass, L3G4200D& gyro)
{
    compass.enableDefault();
    gyro.enableDefault();

    while(1)
    {
        compass.read();
        gyro.read();
        printf("%7d %7d %7d,  %7d %7d %7d,  %7d %7d %7d\n",
               compass.m(0), compass.m(1), compass.m(2),
               compass.a(0), compass.a(1), compass.a(2),
               gyro.g(0), gyro.g(1), gyro.g(2)
        );
        usleep(100*1000);
    }
}

void calibrate(LSM303& compass)
{
    compass.enableDefault();
    int_vector mag_max(0,0,0), mag_min(0,0,0);
    while(1)
    {
        compass.read();
        mag_min = mag_min.cwiseMin(compass.m);
        mag_max = mag_max.cwiseMax(compass.m);
        printf("%7d %7d %7d  %7d %7d %7d\n",
               mag_min(0), mag_min(1), mag_min(2),
               mag_max(0), mag_max(1), mag_max(2));
        usleep(10*1000);
        // TODO: have some way of writing to ~/.lsm303_mag_cal
    }
}

void loadCalibration(int_vector& mag_min, int_vector& mag_max)
{
    // TODO: recalibrate for my new IMU
    // TODO: load from ~/.lsm303_mag_cal instead of hardcoding
    mag_min = int_vector(-835, -931, -978);
    mag_max = int_vector(921, 590, 741);
}

static void enableSensors(LSM303& compass, L3G4200D& gyro)
{
    compass.writeAccReg(LSM303_CTRL_REG1_A, 0x47); // normal power mode, all axes enabled, 50 Hz
    compass.writeAccReg(LSM303_CTRL_REG4_A, 0x20); // 8 g full scale

    compass.writeMagReg(LSM303_MR_REG_M, 0x00); // continuous conversion mode
    // 15 Hz default

    gyro.writeReg(L3G4200D_CTRL_REG1, 0x0F); // normal power mode, all axes enabled, 100 Hz
    gyro.writeReg(L3G4200D_CTRL_REG4, 0x20); // 2000 dps full scale
}

// Calculate offsets, assuming the MiniMU is resting
// with is z axis pointing up.
static void calculateOffsets(LSM303& compass, L3G4200D& gyro,
                             vector& accel_offset, vector& gyro_offset,
                             const int_vector& accel_sign)
{
    // LSM303 accelerometer: 8 g sensitivity.  3.8 mg/digit; 1 g = 256
    const int gravity = 256;

    gyro_offset = accel_offset = vector(0,0,0);
    const int sampleCount = 32;
    for(int i = 0; i < sampleCount; i++)
    {
        gyro.read();
        compass.readAcc();
        gyro_offset += gyro.g.cast<float>();
        accel_offset += compass.a.cast<float>();
        usleep(20*1000);
    }
    gyro_offset /= sampleCount;
    accel_offset /= sampleCount;
    accel_offset(2) -= gravity * accel_sign(2);
}

// Returns the measured angular velocity vector
// in units of radians per second.
static vector readGyro(L3G4200D& gyro, const int_vector& gyro_sign, const vector& gyro_offset)
{
    // At the full-scale=2000 dps setting, the gyro datasheet says
    // we get 0.07 dps/digit.
    const float gyro_gain = 0.07 * 3.14159265 / 180;

    gyro.read();
    return gyro_sign.cast<float>().cwiseProduct( gyro.g.cast<float>() - gyro_offset ) * gyro_gain;
}

static matrix updateMatrix(const vector& w, float dt)
{
    // TODO: represent this in a cooler way, maybe:
    //   http://eigen.tuxfamily.org/dox/classEigen_1_1VectorwiseOp.html#aeaa2c5d72558c2bfc049a098efc25633

    matrix m = matrix::Identity();
    m(2,0) = -w(1) * dt;
    m(0,2) =  w(1) * dt;

    m(0,1) = -w(2) * dt;
    m(1,0) =  w(2) * dt;

    m(1,2) = -w(0) * dt;
    m(2,1) =  w(0) * dt;
    return m;
}

// TODO: change this somehow to treat all the rows equally (currently Z is special)
static matrix normalize(const matrix & m)
{
    float error = m.row(0).dot(m.row(1));
    matrix norm;
    norm.row(0) = m.row(0) - (error/2) * m.row(1);
    norm.row(1) = m.row(1) - (error/2) * m.row(0);
    norm.row(2) = m.row(0).cross(m.row(1));
    return norm;
}

// DCM algorithm: http://diydrones.com/forum/topics/robust-estimator-of-the

void ahrs(LSM303& compass, L3G4200D& gyro)
{
    // X axis pointing forward
    // Y axis pointing to the right 
    // and Z axis pointing down
    // Positive pitch : nose down
    // Positive roll : right wing down
    // Positive yaw : counterclockwise
    const int_vector gyro_sign(1, 1, 1);
    const int_vector accel_sign(1, 1, 1);
    const int_vector mag_sign(1, 1, 1);

    int_vector mag_min, mag_max;
    loadCalibration(mag_min, mag_max);

    enableSensors(compass, gyro);

    vector accel_offset, gyro_offset;
    calculateOffsets(compass, gyro, accel_offset, gyro_offset, accel_sign);
    
    //printf("Offset: %7f %7f %7f  %7f %7f %7f\n",
    //       gyro_offset(0), gyro_offset(1), gyro_offset(2),
    //       accel_offset(0), accel_offset(1), accel_offset(2));

    vector angular_velocity, v_accel, v_magnetom;

    // The rotation matrix that can convert a vector in body coordinates
    // to ground coordinates.
    matrix rotation = matrix::Identity();

    int counter = 0;
    int start = millis(); // truncate 64-bit return value
    while(1)
    {
        int last_start = start;
        start = millis();
        float dt = (start-last_start)/1000.0;
        //printf("dt = %f\n", dt);

        if (dt < 0){ throw "time went backwards"; }       

        angular_velocity = readGyro(gyro, gyro_sign, gyro_offset);
        //readAcc(compass);

        // Every 5 loop runs read compass data (10 Hz)
        if (++counter == 5)
        {
            counter = 0;
            //readMag(compass);
            //compassHeading();
        }

        rotation *= updateMatrix(angular_velocity, dt);
        rotation = normalize(rotation);
        //driftCorrection();
        //eulerAngles();

        //fprintf(stderr, "g: %8d %8d %8d\n", gyro.g(0), gyro.g(1), gyro.g(2));
        fprintf(stderr, "w: %7.4f %7.4f %7.4f\n", angular_velocity(0), angular_velocity(1), angular_velocity(2));

        printf("%7.4f %7.4f %7.4f %7.4f %7.4f %7.4f %7.4f %7.4f %7.4f\n",
               rotation(0,0), rotation(0,1), rotation(0,2),
               rotation(1,0), rotation(1,1), rotation(1,2),
               rotation(2,0), rotation(2,1), rotation(2,2));
        fflush(stdout);

        //std::cout << rotation;

        // Ensure that each iteration of the loop takes at least 20 ms.
        while(millis() - start < 20)
        {
            usleep(1000);
        }
    }
}

int main(int argc, char *argv[])
{
    I2CBus i2c("/dev/i2c-0");
    LSM303 compass(i2c);
    L3G4200D gyro(i2c);

    uint8_t result = compass.readMagReg(LSM303_WHO_AM_I_M);
    if (result != 0x3C)
    {
        std::cerr << "Error getting \"Who Am I\" register." << std::endl;
        exit(2);
    }

    if (argc > 1)
    {
        if (0 == strcmp("cal", argv[1]))
        {
            calibrate(compass);
        }
        else if (0 == strcmp("raw", argv[1]))
        {
            streamRawValues(compass, gyro);
        }
        else
        {
            fprintf(stderr, "Unknown action '%s'.\n", argv[1]);
            exit(3);
        }
    }
    else
    {
        ahrs(compass, gyro);
    }

    return 0;
}
