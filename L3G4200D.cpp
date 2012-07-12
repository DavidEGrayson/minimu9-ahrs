#include "L3G4200D.h"

#define GYR_ADDRESS (0xD6 >> 1)

// TODO: rename gyro class

L3G4200D::L3G4200D(I2CBus& i2c) : i2c(i2c)
{
    // nothing to do here
}

void L3G4200D::address()
{
    i2c.setAddress(GYR_ADDRESS);
}

// Turns on the L3G4200D's gyro and places it in normal mode.
void L3G4200D::enableDefault()
{
    // Normal power mode, all axes enabled
    writeReg(L3G4200D_CTRL_REG1, 0b00001111);
}

void L3G4200D::writeReg(uint8_t reg, uint8_t value)
{
    address();
    i2c.writeByte(reg, value);
}

uint8_t L3G4200D::readReg(uint8_t reg)
{
    address();
    return i2c.readByte(reg);
}

void L3G4200D::read()
{
    address();
    uint8_t block[6];
    i2c.readBlock(0x80 | L3G4200D_OUT_X_L, sizeof(block), block);

    g[0] = (int16_t)(block[1] << 8 | block[0]);
    g[1] = (int16_t)(block[3] << 8 | block[2]);
    g[2] = (int16_t)(block[5] << 8 | block[4]);
}
