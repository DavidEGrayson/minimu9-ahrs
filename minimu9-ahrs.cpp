#include "vector.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "LSM303.h"
#include "L3G4200D.h"
#include <sys/time.h>

/*
promising:
http://elinux.org/Interfacing_with_I2C_Devices
https://i2c.wiki.kernel.org/index.php/Main_Page
*/

// TODO: read http://www.cs.cmu.edu/~spiff/moedit99/expmap.pdf



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
               compass.m[0], compass.m[1], compass.m[2],
               compass.a[0], compass.a[1], compass.a[2],
               gyro.g(0), gyro.g(1), gyro.g(2)
        );
        usleep(100*1000);
    }
}

void calibrate(LSM303& compass)
{
    compass.enableDefault();
    int_vector mag_max, mag_min;
    while(1)
    {
        compass.read();
        mag_min = mag_min.cwiseMin(int_vector_from_ints(&compass.m));
        mag_max = mag_max.cwiseMax(int_vector_from_ints(&compass.m));
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
    mag_min = int_vector(-519, -476, -765);
    mag_max = int_vector(475, 623, 469);
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
                             vector& accel_offset, vector& gyro_offset)
{
    // LSM303 accelerometer: 8 g sensitivity.  3.8 mg/digit; 1 g = 256.
    // TODO: unify this with the other place in the code where we scale accelerometer readings.
    const int gravity = 256;

    gyro_offset = accel_offset = vector::Zero();
    const int sampleCount = 32;
    for(int i = 0; i < sampleCount; i++)
    {
        gyro.read();
        compass.readAcc();
        gyro_offset += gyro.g.cast<float>();
        accel_offset += vector_from_ints(&compass.a);
        usleep(20*1000);
    }
    gyro_offset /= sampleCount;
    accel_offset /= sampleCount;
    accel_offset(2) -= gravity;

    if (accel_offset.norm() > 50)
    {
        fprintf(stderr, "Unable to calculate accelerometer offset because board was not resting in the correct orientation.\n");
        accel_offset = vector::Zero();
    }
}

// Returns the measured angular velocity vector
// in units of radians per second, in the body coordinate system.
static vector readGyro(L3G4200D& gyro, const vector& gyro_offset)
{
    // At the full-scale=2000 dps setting, the gyro datasheet says
    // we get 0.07 dps/digit.
    const float gyro_scale = 0.07 * 3.14159265 / 180;

    gyro.read();
    return ( gyro.g.cast<float>() - gyro_offset ) * gyro_scale;
}

// Returns acceleration vector in units of g, where g is 9.8 m/s^2,
// in the body coordinate system.
static vector readAcc(LSM303& compass, const vector& accel_offset)
{
    // LSM303 accelerometer: At 8 g sensitivity, the datasheet says
    // we get 3.9 mg/digit.
    // TODO: double check this figure using the correct datasheet
    const float accel_scale = 0.0039;

    compass.readAcc();
    return ( vector_from_ints(&compass.a) - accel_offset ) * accel_scale;
}

// Returns the magnetic field vector in the body coordinate system.
// For each component, a value of 1 corresponds to the max value
// and a value of -1 corresponds to the min value.
static vector readMag(LSM303& compass, const int_vector& mag_min, const int_vector& mag_max)
{
    compass.readMag();
    vector m;
    m(0) = (float)(compass.m[0] - mag_min(0)) / (mag_max(0) - mag_min(0)) * 2 - 1;
    m(1) = (float)(compass.m[1] - mag_min(1)) / (mag_max(1) - mag_min(1)) * 2 - 1;
    m(2) = (float)(compass.m[2] - mag_min(2)) / (mag_max(2) - mag_min(2)) * 2 - 1;
    return m;
}

static matrix updateMatrix(const vector& w, float dt)
{
    matrix u = matrix::Identity();
    u(2,0) = -w(1) * dt;
    u(0,2) =  w(1) * dt;

    u(0,1) = -w(2) * dt;
    u(1,0) =  w(2) * dt;

    u(1,2) = -w(0) * dt;
    u(2,1) =  w(0) * dt;
    return u;
}

static matrix normalize(const matrix & m)
{
    //float error = m.row(0).dot(m.row(1));
    matrix norm;
    norm.row(0) = m.row(0) + m.row(1).cross(m.row(2))/10;
    norm.row(1) = m.row(1) + m.row(2).cross(m.row(0))/10;
    norm.row(2) = m.row(2) + m.row(0).cross(m.row(1))/10;
    norm.row(0).normalize();
    norm.row(1).normalize();
    norm.row(2).normalize();
    return norm;
}

void fuse_gyro_only(matrix& rotation, float dt, const vector& angular_velocity)
{
   rotation *= updateMatrix(angular_velocity, dt);

   rotation = normalize(rotation);
}

#define Kp_ROLLPITCH 1
#define Ki_ROLLPITCH 0.00002
#define Kp_YAW 1
#define Ki_YAW 0.00002

#define PI 3.14159265

void fuse(matrix& rotation, float dt, const vector& angular_velocity,
  const vector& acceleration, const vector& magnetic_field)
{
    vector up = acceleration;     // usually true
    vector magnetic_east = magnetic_field.cross(up);
    vector east = magnetic_east;  // a rough approximation
    vector north = up.cross(east);

    matrix rotationFromCompass;
    rotationFromCompass.row(0) = east;
    rotationFromCompass.row(1) = north;
    rotationFromCompass.row(2) = up;
    rotationFromCompass.row(0).normalize();
    rotationFromCompass.row(1).normalize();
    rotationFromCompass.row(2).normalize();

    // The board's x axis in earth coordinates.
    vector x = rotationFromCompass.col(0);
    x.normalize();
    x(2) = 0;
    float heading_weight = x.norm();

    // 0 = east, pi/2 = north
    float heading = atan2(x(1), x(0));

    fprintf(stderr, "B = %7.4f %7.4f %7.4f (%7.4f)  w=%7.4f h=%7.4f\n",
            magnetic_field(0), magnetic_field(1), magnetic_field(2),
            magnetic_field.norm(), heading_weight, heading*180/PI);

    // We trust the accelerometer more if it is telling us 1G.
    float accel_weight = 1 - 2*abs(1 - acceleration.norm());
    if (accel_weight < 0){ accel_weight = 0; }

    vector omegaP(0,0,0);
    //static vector omegaI;

    // Add a "torque" that makes our up vector (rotation.row(2))
    // get closer to the acceleration vector.
    vector errorRollPitch = acceleration.cross(rotation.row(2)) * accel_weight;

    //fprintf(stderr, "errorRollPitch = %7.4f %7.4f %7.4f\n", errorRollPitch(0), errorRollPitch(1), errorRollPitch(2));

    omegaP += errorRollPitch * Kp_ROLLPITCH;
    //omegaI += errorRollPitch * Ki_ROLLPITCH;

    // Add a "torque" that makes our east vector (rotation.row(0))
    // get closer to east vector calculated from the compass.
    vector errorYaw = east.cross(rotation.row(0));
    omegaP += errorYaw * Kp_YAW;
    //omegaI += errorYaw * Ki_YAW;

    rotation *= updateMatrix(angular_velocity + omegaP, dt);
    rotation = normalize(rotation);
}

// DCM algorithm: http://diydrones.com/forum/topics/robust-estimator-of-the

void print(matrix m)
{
    printf("%7.4f %7.4f %7.4f %7.4f %7.4f %7.4f %7.4f %7.4f %7.4f",
           m(0,0), m(0,1), m(0,2),
           m(1,0), m(1,1), m(1,2),
           m(2,0), m(2,1), m(2,2));
}

void ahrs(LSM303& compass, L3G4200D& gyro)
{
    int_vector mag_min, mag_max;
    loadCalibration(mag_min, mag_max);

    enableSensors(compass, gyro);

    vector accel_offset, gyro_offset;
    calculateOffsets(compass, gyro, accel_offset, gyro_offset);
    
    fprintf(stderr, "Gyro offset: %7f %7f %7f\nAccel offset: %7f %7f %7f (%7f)\n",
            gyro_offset(0), gyro_offset(1), gyro_offset(2),
            accel_offset(0), accel_offset(1), accel_offset(2),
            accel_offset.norm());

    // The rotation matrix that can convert a vector in body coordinates
    // to ground coordinates.
    matrix rotation = matrix::Identity();

    int start = millis(); // truncate 64-bit return value
    while(1)
    {
        int last_start = start;
        start = millis();
        float dt = (start-last_start)/1000.0;
        if (dt < 0){ throw "time went backwards"; }

        vector angular_velocity = readGyro(gyro, gyro_offset);
        vector acceleration = readAcc(compass, accel_offset);
        vector magnetic_field = readMag(compass, mag_min, mag_max); // TODO: read mag at 10Hz instead?  Why do others do that?

        fuse(rotation, dt, angular_velocity, acceleration, magnetic_field);

        //fprintf(stderr, "g: %8d %8d %8d\n", gyro.g(0), gyro.g(1), gyro.g(2));
        //fprintf(stderr, "m: %7.4f %7.4f %7.4f  m_raw: %8d %8d %8d\n", magnetic_field(0), magnetic_field(1), magnetic_field(2), compass.m(0), compass.m(1), compass.m(2)); 
        //fprintf(stderr, "dt: %7.4f  w: %7.4f %7.4f %7.4f\n", dt, angular_velocity(0), angular_velocity(1), angular_velocity(2));

        //rotation = rotationFromCompass;  // TMPHAX

        printf("%7.4f %7.4f %7.4f %7.4f %7.4f %7.4f %7.4f %7.4f %7.4f  %7.4f %7.4f %7.4f  %7.4f %7.4f %7.4f\n",
               rotation(0,0), rotation(0,1), rotation(0,2),
               rotation(1,0), rotation(1,1), rotation(1,2),
               rotation(2,0), rotation(2,1), rotation(2,2),
               acceleration(0), acceleration(1), acceleration(2),
               magnetic_field(0), magnetic_field(1), magnetic_field(2));
        fflush(stdout);

        //std::cout << rotation;

        // Ensure that each iteration of the loop takes at least 20 ms.
        while(millis() - start < 20)
        {
            usleep(1000);
        }
    }
}

void tmphaxTest()
{
    const float turnAmount = 3.14159265/2;  // radians
    const float turnTime = 0.5; // seconds
    const int pieces = turnTime/0.02;
    const vector angular_velocity(0, 0, turnAmount/turnTime);

    const float dt = turnTime/pieces;
    matrix rotation = matrix::Identity();

    for (int i = 0; i < pieces; i++)
    {
        fuse_gyro_only(rotation, dt, angular_velocity);
    }

    print(rotation);
    putchar('\n');
    printf("tmphaxTest done\n");
    exit(3);
}

int main(int argc, char *argv[])
{
    I2CBus i2c("/dev/i2c-0");
    LSM303 compass(i2c);
    L3G4200D gyro(i2c);

    //tmphaxTest();

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
