#include "lps25h.h"
#include <stdexcept>

void lps25h::handle::open(const comm_config & config)
{
  if (!config.use_sensor)
  {
    throw std::runtime_error("LPS25H configuration is null.");
  }

  this->config = config;
  i2c.open(config.i2c_bus_name);
}

void lps25h::handle::enable()
{
  if (config.device == LPS25H)
  {
    //// LPS25H altimeter

    // Trun devices off
    write_reg(CTRL_REG1, 0x00);

    // Turn devices on
    write_reg(CTRL_REG1, 0b10110100);
    // Configure, set defaults:
    write_reg(CTRL_REG2, 0x00);

    uint8_t xlow;
    uint8_t low;
    uint8_t high;
    i2c_write_byte_and_read(config.i2c_address, REF_P_XL, xlow, sizeof(xlow));
    i2c.write_byte_and_read(config.i2c_address, REF_P_L, low, sizeof(low));
    i2c.write_byte_and_read(config.i2c_address, REF_P_H, high, sizeof(high));
    refp = (int32_t)(xlow | low << 8 | high << 16);
  }
  else
  {
    throw std::runtime_error("Cannot enable unknown device.");
  }
}

void lps25h::handle::write_reg(reg_addr addr, uint8_t value)
{
  i2c.write_two_bytes(config.i2c_address, addr, value);
}

void lps25h::handle::read_temp()
{
  uint8_t low;
  uint8_t high;
  i2c.write_byte_and_read(config.i2c_address, TEMP_OUT_L, low, sizeof(low));
  i2c.write_byte_and_read(config.i2c_address, TEMP_OUT_H, high, sizeof(high));
  t = (int16_t)(low | high << 8);
}

void lps25h::handle::read_press()
{
  uint8_t xlow;
  uint8_t low;
  uint8_t high;
  i2c_write_byte_and_read(config.i2c_address, PRESS_OUT_XL, xlow, sizeof(xlow));
  i2c.write_byte_and_read(config.i2c_address, PRESS_OUT_L, low, sizeof(low));
  i2c.write_byte_and_read(config.i2c_address, PRESS_OUT_H, high, sizeof(high));
  p = (int32_t)(xlow | low << 8 | high << 16);
}

