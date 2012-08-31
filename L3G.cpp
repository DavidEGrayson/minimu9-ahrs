#include "L3G.h"
#include <stdexcept>

#define L3G4200D_ADDRESS_SA0_LOW  (0xD0 >> 1)
#define L3G4200D_ADDRESS_SA0_HIGH (0xD2 >> 1)
#define L3GD20_ADDRESS_SA0_LOW    (0xD4 >> 1)
#define L3GD20_ADDRESS_SA0_HIGH   (0xD6 >> 1)

L3G::L3G(const char * i2cDeviceName) : i2c(i2cDeviceName)
{
    detectAddress();
}

void L3G::detectAddress()
{
    i2c.addressSet(L3G4200D_ADDRESS_SA0_LOW);
    if (i2c.tryReadByte(L3G_WHO_AM_I) == 0xD3) return;
    i2c.addressSet(L3G4200D_ADDRESS_SA0_HIGH);
    if (i2c.tryReadByte(L3G_WHO_AM_I) == 0xD3) return;
    i2c.addressSet(L3GD20_ADDRESS_SA0_LOW);
    if (i2c.tryReadByte(L3G_WHO_AM_I) == 0xD4) return;
    i2c.addressSet(L3GD20_ADDRESS_SA0_HIGH);
    if (i2c.tryReadByte(L3G_WHO_AM_I) == 0xD4) return;

    throw std::runtime_error("Could not detect gyro.");
}

// Turns on the gyro and places it in normal mode.
void L3G::enable()
{
    writeReg(L3G_CTRL_REG1, 0b00001111); // Normal power mode, all axes enabled
    writeReg(L3G_CTRL_REG4, 0b00100000); // 2000 dps full scale
}

void L3G::writeReg(uint8_t reg, uint8_t value)
{
    i2c.writeByte(reg, value);
}

uint8_t L3G::readReg(uint8_t reg)
{
    return i2c.readByte(reg);
}

void L3G::read()
{
    uint8_t block[6];
    i2c.readBlock(0x80 | L3G_OUT_X_L, sizeof(block), block);

    g[0] = (int16_t)(block[1] << 8 | block[0]);
    g[1] = (int16_t)(block[3] << 8 | block[2]);
    g[2] = (int16_t)(block[5] << 8 | block[4]);
}
