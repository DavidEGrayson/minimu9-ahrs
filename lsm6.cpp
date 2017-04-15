#include "lsm6.h"
#include <stdexcept>

void lsm6::handle::open(const comm_config & config)
{
  if (!config.use_sensor)
  {
    throw std::runtime_error("LSM6 configuration is null.");
  }

  this->config = config;
  i2c.open(config.i2c_bus_name);
}

void lsm6::handle::enable()
{
  if (config.device == LSM6DS33)
  {
    //// LSM6DS33 gyro

    // ODR = 1000 (1.66 kHz (high performance))
    // FS_G = 11 (2000 dps)
    write_reg(CTRL2_G, 0b10001100);

    // defaults
    write_reg(CTRL7_G, 0b00000000);

    //// LSM6DS33 accelerometer

    // ODR = 1000 (1.66 kHz (high performance))
    // FS_XL = 11 (8 g full scale)
    // BW_XL = 00 (400 Hz filter bandwidth)
    write_reg(CTRL1_XL, 0b10001100);

    //// common

    // IF_INC = 1 (automatically increment address register)
    write_reg(CTRL3_C, 0b00000100);
  }
  else
  {
    throw std::runtime_error("Cannot enable unknown LSM6 device.");
  }
}

void lsm6::handle::write_reg(reg_addr addr, uint8_t value)
{
  i2c.write_two_bytes(config.i2c_address, addr, value);
}

void lsm6::handle::read_gyro()
{
  uint8_t block[6];
  i2c.write_byte_and_read(config.i2c_address,
    OUTX_L_G, block, sizeof(block));
  g[0] = (int16_t)(block[0] | block[1] << 8);
  g[1] = (int16_t)(block[2] | block[3] << 8);
  g[2] = (int16_t)(block[4] | block[5] << 8);
}

void lsm6::handle::read_acc()
{
  uint8_t block[6];
  i2c.write_byte_and_read(config.i2c_address,
    OUTX_L_XL, block, sizeof(block));
  a[0] = (int16_t)(block[0] | block[1] << 8);
  a[1] = (int16_t)(block[2] | block[3] << 8);
  a[2] = (int16_t)(block[4] | block[5] << 8);
}

