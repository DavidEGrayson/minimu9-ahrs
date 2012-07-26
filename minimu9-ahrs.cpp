#include "vector.h"
#include "MinIMU9.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

int millis()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec) * 1000 + (tv.tv_usec)/1000;
}

void streamRawValues(IMU& imu)
{
    imu.enableSensors();

    while(1)
    {
        imu.read();
        printf("%7d %7d %7d,  %7d %7d %7d,  %7d %7d %7d\n",
               imu.m[0], imu.m[1], imu.m[2],
               imu.a[0], imu.a[1], imu.a[2],
               imu.g[0], imu.g[1], imu.g[2]
        );
        usleep(20*1000);
    }
}

void calibrateMag(IMU& imu)
{
    imu.enableSensors();
    int_vector mag_max, mag_min;
    while(1)
    {
        imu.readMag();
        mag_min = mag_min.cwiseMin(imu.m);
        mag_max = mag_max.cwiseMax(imu.m);
        printf("%7d %7d %7d  %7d %7d %7d\n",
               mag_min(0), mag_min(1), mag_min(2),
               mag_max(0), mag_max(1), mag_max(2));
        usleep(10*1000);
        // TODO: have some way of writing to ~/.lsm303_mag_cal
    }
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

void ahrs(IMU& imu)  // TODO: change this to just be IMU& eventually
{
    imu.loadCalibration();
    imu.enableSensors();
    imu.measureOffsets();
    
    fprintf(stderr, "Gyro offset: %7f %7f %7f\nAccel offset: %7f %7f %7f (%7f)\n",
            imu.gyro_offset(0), imu.gyro_offset(1), imu.gyro_offset(2),
            imu.accel_offset(0), imu.accel_offset(1), imu.accel_offset(2),
            imu.accel_offset.norm());

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

        vector angular_velocity = imu.readGyro();
        vector acceleration = imu.readAcc();
        vector magnetic_field = imu.readMag();

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

int main(int argc, char *argv[])
{
    I2CBus i2c("/dev/i2c-0");
    MinIMU9 imu(i2c);

    imu.checkConnection();

    if (argc > 1)
    {
        if (0 == strcmp("cal", argv[1]))
        {
            calibrateMag(imu);
        }
        else if (0 == strcmp("raw", argv[1]))
        {
            streamRawValues(imu);
        }
        else
        {
            fprintf(stderr, "Unknown action '%s'.\n", argv[1]);
            exit(3);
        }
    }
    else
    {
        ahrs(imu);
    }

    return 0;
}
