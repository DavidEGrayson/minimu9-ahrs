#include "lis3mdl.h"
#include <stdexcept>

void lis3mdl::handle::open(const comm_config & config)
{
  if (!config.use_sensor)
  {
    throw std::runtime_error("LIS3MDL configuration is null.");
  }

  this->config = config;
  i2c.open(config.i2c_bus_name);
}

void lis3mdl::handle::write_reg(reg_addr addr, uint8_t value)
{
  i2c.write_two_bytes(config.i2c_address, addr, value);
}

void lis3mdl::handle::enable()
{
  if (config.device == LIS3MDL)
  {
    // OM = 11 (ultra-high-performance mode for X and Y); DO = 100 (10 Hz ODR)
    write_reg(CTRL_REG1, 0b01110000);

    // FS = 00 (+/- 4 gauss full scale)
    write_reg(CTRL_REG2, 0b00000000);

    // MD = 00 (continuous-conversion mode)
    write_reg(CTRL_REG3, 0b00000000);

    // OMZ = 11 (ultra-high-performance mode for Z)
    write_reg(CTRL_REG4, 0b00001100);
  }
  else
  {
    std::runtime_error("Cannot enable unknown LIS3MDL device.");
  }
}

void lis3mdl::handle::read()
{
  uint8_t block[6];
  i2c.write_byte_and_read(config.i2c_address,
    0x80 | OUT_X_L, block, sizeof(block));
  m[0] = (int16_t)(block[0] | block[1] << 8);
  m[1] = (int16_t)(block[2] | block[3] << 8);
  m[2] = (int16_t)(block[4] | block[5] << 8);
}
