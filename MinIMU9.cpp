#include "vector.h"
#include "MinIMU9.h"
#include <stdio.h>

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
    mag_min = int_vector(-519, -476, -765);
    mag_max = int_vector(475, 623, 469);
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
    return vector(44,44,44); // TODO: this
}

vector MinIMU9::readAcc()
{
    return vector(44,44,44); // TODO: this
}

vector MinIMU9::readGyro()
{
    return vector(44,44,44); // TODO: this
}
