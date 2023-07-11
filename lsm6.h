#pragma once

#include <i2c_bus.h>
#include <cstdint>

namespace lsm6
{
  enum device_type
  {
    LSM6DS33 = 0x69,
    LSM6DSO = 0x6C,
  };

  enum i2c_addr
  {
    SA0_LOW_ADDR = 0x6A,
    SA0_HIGH_ADDR = 0x6B,
  };

  enum reg_addr
  {
    INT1_CTRL         = 0x0D,
    INT2_CTRL         = 0x0E,
    WHO_AM_I          = 0x0F,
    CTRL1_XL          = 0x10,
    CTRL2_G           = 0x11,
    CTRL3_C           = 0x12,
    CTRL4_C           = 0x13,
    CTRL5_C           = 0x14,
    CTRL6_C           = 0x15,
    CTRL7_G           = 0x16,
    CTRL8_XL          = 0x17,
    CTRL9_XL          = 0x18,
    CTRL10_C          = 0x19,

    STATUS_REG        = 0x1E,

    OUT_TEMP_L        = 0x20,
    OUT_TEMP_H        = 0x21,
    OUTX_L_G          = 0x22,
    OUTX_H_G          = 0x23,
    OUTY_L_G          = 0x24,
    OUTY_H_G          = 0x25,
    OUTZ_L_G          = 0x26,
    OUTZ_H_G          = 0x27,
    OUTX_L_XL         = 0x28,
    OUTX_H_XL         = 0x29,
    OUTY_L_XL         = 0x2A,
    OUTY_H_XL         = 0x2B,
    OUTZ_L_XL         = 0x2C,
    OUTZ_H_XL         = 0x2D,

    FIFO_STATUS1      = 0x3A,
    FIFO_STATUS2      = 0x3B,
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

    void read_gyro();
    void read_acc();

    // gyro angular velocity readings
    int32_t g[3];

    // acceleration readings
    int32_t a[3];

  protected:
    i2c_bus i2c;
    comm_config config;
  };
};
