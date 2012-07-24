#include "MinIMU9.h"

MinIMU9::MinIMU9(I2CBus& i2c) : compass(i2c), gyro(i2c)
{
}

MinIMU9::enableSensors()
{
    compass.writeAccReg(LSM303_CTRL_REG1_A, 0x47); // normal power mode, all axes enabled, 50 Hz
    compass.writeAccReg(LSM303_CTRL_REG4_A, 0x20); // 8 g full scale

    compass.writeMagReg(LSM303_MR_REG_M, 0x00); // continuous conversion mode
    // 15 Hz default

    gyro.writeReg(L3G_CTRL_REG1, 0x0F); // normal power mode, all axes enabled, 100 Hz
    gyro.writeReg(L3G_CTRL_REG4, 0x20); // 2000 dps full scale
}

