#include "L3G.h"

#define GYR_ADDRESS (0xD6 >> 1)

// TODO: rename gyro class

L3G::L3G(I2CBus& i2c) : i2c(i2c)
{
    fgyro = i2c.registerI2CDevice(GYR_ADDRESS);
}

L3G::~L3G()
{
    i2c.deregisterI2CDevice(fgyro);
}

// Turns on the gyro and places it in normal mode.
void L3G::enableDefault()
{
    // Normal power mode, all axes enabled
    writeReg(L3G_CTRL_REG1, 0b00001111);
}

void L3G::writeReg(uint8_t reg, uint8_t value)
{
//    address();
    i2c.writeByte(fgyro, reg, value);
}

uint8_t L3G::readReg(uint8_t reg)
{
//    address();
    return i2c.readByte(fgyro, reg);
}

void L3G::read()
{
//    address();
    uint8_t block[6];
    i2c.readBlock(fgyro, 0x80 | L3G_OUT_X_L, sizeof(block), block);

    g[0] = (int16_t)(block[1] << 8 | block[0]);
    g[1] = (int16_t)(block[3] << 8 | block[2]);
    g[2] = (int16_t)(block[5] << 8 | block[4]);
}
