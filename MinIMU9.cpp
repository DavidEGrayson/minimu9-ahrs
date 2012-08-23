#include "vector.h"
#include "MinIMU9.h"
#include <stdio.h>
#include <iostream>
#include <fstream>

MinIMU9::MinIMU9(I2CBus& i2c) : compass(i2c), gyro(i2c)
{
}

void MinIMU9::checkConnection()
{
    uint8_t result = compass.readMagReg(LSM303_WHO_AM_I_M);
    if (result != 0x3C)
    {
        fprintf(stderr, "Error getting \"Who Am I\" register.\n");
        exit(2);
    }
}

void MinIMU9::enableSensors()
{
    //compass.enableDefault();
    //gyro.enableDefault();

    compass.writeAccReg(LSM303_CTRL_REG1_A, 0x47); // normal power mode, all axes enabled, 50 Hz
    compass.writeAccReg(LSM303_CTRL_REG4_A, 0x20); // 8 g full scale

    compass.writeMagReg(LSM303_MR_REG_M, 0x00); // continuous conversion mode
    // 15 Hz default

    gyro.writeReg(L3G_CTRL_REG1, 0x0F); // normal power mode, all axes enabled, 100 Hz
    gyro.writeReg(L3G_CTRL_REG4, 0x20); // 2000 dps full scale
}

void MinIMU9::loadCalibration()
{
    // TODO: load from ~/.lsm303_mag_cal instead of hardcoding

    // On table with projector:
    //mag_min = int_vector(-525, -746, -728);
    //mag_max = int_vector(737, 665, 633);

    // On floor near TV stand:
    //mag_min = int_vector(-421, -563, -462);
    //mag_max = int_vector( 537,  479,  481);

    std::ifstream file("~/.minimu9-ahrs-cal");
    file >> mag_min(0) >> mag_max(0) >> mag_min(1) >> mag_max(1) >> mag_min(2) >> mag_max(2);
    if (file.fail() || file.bad())
    {
        throw "Failed to read calibration file ~/.minimu9-ahrs-cal.";
    }
}

// Calculate offsets, assuming the MinIMU is resting
// with is z axis pointing up.
void MinIMU9::measureOffsets()
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
        gyro_offset += vector_from_ints(&gyro.g);
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

vector MinIMU9::readMag()
{
    compass.readMag();
    IMU::m = int_vector_from_ints(&compass.m);
    
    vector v;
    v(0) = (float)(compass.m[0] - mag_min(0)) / (mag_max(0) - mag_min(0)) * 2 - 1;
    v(1) = (float)(compass.m[1] - mag_min(1)) / (mag_max(1) - mag_min(1)) * 2 - 1;
    v(2) = (float)(compass.m[2] - mag_min(2)) / (mag_max(2) - mag_min(2)) * 2 - 1;
    return v;
}

vector MinIMU9::readAcc()
{
    // LSM303 accelerometer: At 8 g sensitivity, the datasheet says
    // we get 3.9 mg/digit.
    // TODO: double check this figure using the correct datasheet
    const float accel_scale = 0.0039;

    compass.readAcc();
    IMU::a = int_vector_from_ints(&compass.a);
    return ( vector_from_ints(&compass.a) - accel_offset ) * accel_scale;
}

vector MinIMU9::readGyro()
{
    // At the full-scale=2000 dps setting, the gyro datasheet says
    // we get 0.07 dps/digit.
    const float gyro_scale = 0.07 * 3.14159265 / 180;

    gyro.read();
    IMU::g = int_vector_from_ints(&gyro.g);
    return ( vector_from_ints(&gyro.g) - gyro_offset ) * gyro_scale;
}
