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
    // TODO: load from ~/.lsm303_mag_cal instead of hardcoding
    mag_min = int_vector(-835, -931, -978);
    mag_max = int_vector(921, 590, 741);
}

// LSM303 accelerometer: 8 g sensitivity.  3.8 mg/digit; 1 g = 256
const int gravity = 256;

// X axis pointing forward
// Y axis pointing to the left 
// and Z axis pointing up
// Positive pitch : nose down
// Positive roll : right wing down
// Positive yaw : counterclockwise
// TODO: really understand what these sign vectors do
const int_vector gyro_sign(1, -1, -1);
const int_vector accel_sign(-1, 1, 1);
const int_vector mag_sign(1, -1, -1);

vector gyro_offset(0,0,0), accel_offset(0,0,0);

void ahrs(LSM303& compass, L3G4200D& gyro)
{
    int_vector mag_min, mag_max;
    loadCalibration(mag_min, mag_max);

    compass.writeAccReg(LSM303_CTRL_REG1_A, 0x47); // normal power mode, all axes enabled, 50 Hz
    compass.writeAccReg(LSM303_CTRL_REG4_A, 0x20); // 8 g full scale

    compass.writeMagReg(LSM303_MR_REG_M, 0x00); // continuous conversion mode
    // 15 Hz default

    gyro.writeReg(L3G4200D_CTRL_REG1, 0x0F); // normal power mode, all axes enabled, 100 Hz
    gyro.writeReg(L3G4200D_CTRL_REG4, 0x20); // 2000 dps full scale

    // Calculate offsets, assuming the MiniMU is resting
    // with is z acis pointing up.
    const int sampleCount = 32;
    for(int i = 0; i < 32; i++)
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

    printf("Offset: %7f %7f %7f  %7f %7f %7f\n",
           gyro_offset(0), gyro_offset(1), gyro_offset(2),
           accel_offset(0), accel_offset(1), accel_offset(2));

    // TODO: better timing system that won't randomly drift
    int counter = 0;
    int start = millis(); // truncate 64-bit return value
    while(1)
    {
        int last_start = start;
        start = millis();
        float dt = (start-last_start)/1000.0;
        printf("dt = %f\n", dt);

        if (dt < 0){ throw "time went backwards"; }       

        // Every 5 loop runs read compass data.
        if (counter > 5)
        {
            counter = 0;
        }

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
