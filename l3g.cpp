#include "l3g.h"
#include <stdexcept>

l3g::l3g(const std::string & i2cDeviceName) : i2c(i2cDeviceName)
{
  detectAddress();
}

void l3g::detectAddress()
{
  // TODO: remove

  int whoami;

  if (i2c.try_write_byte_and_read_byte(l3g::L3G4200D_SA0_LOW_ADDR, WHO_AM_I) == 0xD3)
  {
    // Detected L3G4200D with the SA0 pin low.
    address = l3g::L3G4200D_SA0_LOW_ADDR;
    return;
  }

  if (i2c.try_write_byte_and_read_byte(l3g::L3G4200D_SA0_HIGH_ADDR, WHO_AM_I) == 0xD3)
  {
    // Detected L3G4200D with the SA0 pin high.
    address = l3g::L3G4200D_SA0_HIGH_ADDR;
    return;
  }

  whoami = i2c.try_write_byte_and_read_byte(l3g::L3GD20_SA0_LOW_ADDR, WHO_AM_I);
  if (whoami == 0xD4)
  {
    // Detected L3GD20 with the SA0 pin low.
    address = l3g::L3GD20_SA0_LOW_ADDR;
    return;
  }
  if (whoami == 0xD7)
  {
    // Detected L3GD20H with the SA0 pin low.
    address = l3g::L3GD20_SA0_LOW_ADDR;
    return;
  }

  whoami = i2c.try_write_byte_and_read_byte(l3g::L3GD20_SA0_HIGH_ADDR, WHO_AM_I);
  if (whoami == 0xD4)
  {
    // Detected L3GD20 with the SA0 pin high.
    address = l3g::L3GD20_SA0_HIGH_ADDR;
    return;
  }
  if (whoami == 0xD7)
  {
    // Detected L3GD20H with the SA0 pin high.
    address = l3g::L3GD20_SA0_HIGH_ADDR;
    return;
  }

  throw std::runtime_error("Could not detect gyro.");
}

// Turns on the gyro and places it in normal mode.
void l3g::enable()
{
  writeReg(CTRL_REG1, 0b00001111); // Normal power mode, all axes enabled
  writeReg(CTRL_REG4, 0b00100000); // 2000 dps full scale
}

void l3g::writeReg(uint8_t reg, uint8_t value)
{
  i2c.write_two_bytes(address, reg, value);
}

uint8_t l3g::readReg(uint8_t reg)
{
  return i2c.write_byte_and_read_byte(address, reg);
}

void l3g::read()
{
  uint8_t block[6];
  i2c.write_byte_and_read(address, 0x80 | OUT_X_L, block, sizeof(block));

  g[0] = (int16_t)(block[1] << 8 | block[0]);
  g[1] = (int16_t)(block[3] << 8 | block[2]);
  g[2] = (int16_t)(block[5] << 8 | block[4]);
}
