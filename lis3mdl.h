#pragma once

#include <i2c_bus.h>
#include <cstdint>

namespace lis3mdl
{
  enum device_type
  {
    LIS3MDL = 0x3D,
  };

  enum i2c_addr
  {
    SA1_LOW_ADDR = 0x1E,
    SA1_HIGH_ADDR = 0x1C,
  };

  enum reg_addr
  {
    WHO_AM_I    = 0x0F,

    CTRL_REG1   = 0x20,
    CTRL_REG2   = 0x21,
    CTRL_REG3   = 0x22,
    CTRL_REG4   = 0x23,
    CTRL_REG5   = 0x24,

    STATUS_REG  = 0x27,
    OUT_X_L     = 0x28,
    OUT_X_H     = 0x29,
    OUT_Y_L     = 0x2A,
    OUT_Y_H     = 0x2B,
    OUT_Z_L     = 0x2C,
    OUT_Z_H     = 0x2D,
    TEMP_OUT_L  = 0x2E,
    TEMP_OUT_H  = 0x2F,
    INT_CFG     = 0x30,
    INT_SRC     = 0x31,
    INT_THS_L   = 0x32,
    INT_THS_H   = 0x33,
  };

  struct comm_config {
    bool use_sensor = false;
    device_type device;
    std::string i2c_bus_name;
    i2c_addr i2c_address;
  };

  class handle
  {
  public:
    void open(const comm_config &);

    void enable();

    void write_reg(reg_addr addr, uint8_t value);

    void read();

    int32_t m[3];

  protected:
    i2c_bus i2c;
    comm_config config;
  };
};
