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
