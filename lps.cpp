#include "lps.h"
#include <stdexcept>

void lps::handle::open(const comm_config & config)
{
  if (!config.use_sensor)
  {
    throw std::runtime_error("LPS configuration is null.");
  }

  this->config = config;
  i2c.open(config.i2c_bus_name);
}

void lps::handle::enable()
{
  if (config.device == LPS22DF)
  {
    // from https://github.com/pololu/lps-arduino/blob/master/LPS.cpp
    write_reg(CTRL_REG1, 0x18);
    write_reg(CTRL_REG3, 0x01);
  }
  else
  {
    throw std::runtime_error("Cannot enable unknown LPS device.");
  }
}

void lps::handle::write_reg(reg_addr addr, uint8_t value)
{
  i2c.write_two_bytes(config.i2c_address, addr, value);
}

void lps::handle::read_pressure()
{
  uint8_t b[3];
  i2c.write_byte_and_read(config.i2c_address, PRESS_OUT_XL, b, sizeof(b));
  pressure_raw = (int32_t)(int8_t)b[2] << 16 | b[1] << 8 | b[0];
  pressure_inches_hg = (float)pressure_raw / 138706.5;
}
