#include "L3G.h"
#include <stdexcept>

#define L3G4200D_ADDRESS_SA0_LOW  (0xD0 >> 1)
#define L3G4200D_ADDRESS_SA0_HIGH (0xD2 >> 1)
#define L3GD20_ADDRESS_SA0_LOW    (0xD4 >> 1)
#define L3GD20_ADDRESS_SA0_HIGH   (0xD6 >> 1)

L3G::L3G(const std::string & i2cDeviceName) : i2c(i2cDeviceName)
{
    detectAddress();
}

void L3G::detectAddress()
{
  int whoami;

  if (i2c.try_write_byte_and_read_byte(L3G4200D_ADDRESS_SA0_LOW, L3G_WHO_AM_I) == 0xD3)
  {
    // Detected L3G4200D with the SA0 pin low.
    address = L3G4200D_ADDRESS_SA0_LOW;
    return;
  }

  if (i2c.try_write_byte_and_read_byte(L3G4200D_ADDRESS_SA0_HIGH, L3G_WHO_AM_I) == 0xD3)
  {
    // Detected L3G4200D with the SA0 pin high.
    address = L3G4200D_ADDRESS_SA0_HIGH;
    return;
  }

  whoami = i2c.try_write_byte_and_read_byte(L3GD20_ADDRESS_SA0_LOW, L3G_WHO_AM_I);
  if (whoami == 0xD4)
  {
    // Detected L3GD20 with the SA0 pin low.
    address = L3GD20_ADDRESS_SA0_LOW;
    return;
  }
  if (whoami == 0xD7)
  {
    // Detected L3GD20H with the SA0 pin low.
    address = L3GD20_ADDRESS_SA0_LOW;
    return;
  }

  whoami = i2c.try_write_byte_and_read_byte(L3GD20_ADDRESS_SA0_HIGH, L3G_WHO_AM_I);
  if (whoami == 0xD4)
  {
    // Detected L3GD20 with the SA0 pin high.
    address = L3GD20_ADDRESS_SA0_HIGH;
    return;
  }
  if (whoami == 0xD7)
  {
    // Detected L3GD20H with the SA0 pin high.
    address = L3GD20_ADDRESS_SA0_HIGH;
    return;
  }

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
  i2c.write_two_bytes(address, reg, value);
}

uint8_t L3G::readReg(uint8_t reg)
{
  return i2c.write_byte_and_read_byte(address, reg);
}

void L3G::read()
{
  uint8_t block[6];
  i2c.write_byte_and_read(address, 0x80 | L3G_OUT_X_L, block, sizeof(block));

  g[0] = (int16_t)(block[1] << 8 | block[0]);
  g[1] = (int16_t)(block[3] << 8 | block[2]);
  g[2] = (int16_t)(block[5] << 8 | block[4]);
}
