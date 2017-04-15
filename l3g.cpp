#include "l3g.h"
#include <stdexcept>

void l3g::handle::open(const comm_config & config)
{
  if (!config.use_sensor)
  {
    throw std::runtime_error("L3G configuration is null.");
  }

  this->config = config;
  i2c.open(config.i2c_bus_name);
}

// Turns on the gyro and places it in normal mode.
void l3g::handle::enable()
{
  write_reg(CTRL_REG1, 0b00001111); // Normal power mode, all axes enabled
  write_reg(CTRL_REG4, 0b00100000); // 2000 dps full scale
}

void l3g::handle::write_reg(uint8_t reg, uint8_t value)
{
  i2c.write_two_bytes(config.i2c_address, reg, value);
}

uint8_t l3g::handle::read_reg(uint8_t reg)
{
  return i2c.write_byte_and_read_byte(config.i2c_address, reg);
}

void l3g::handle::read()
{
  uint8_t block[6];
  i2c.write_byte_and_read(config.i2c_address,
    0x80 | OUT_X_L, block, sizeof(block));

  g[0] = (int16_t)(block[1] << 8 | block[0]);
  g[1] = (int16_t)(block[3] << 8 | block[2]);
  g[2] = (int16_t)(block[5] << 8 | block[4]);
}
