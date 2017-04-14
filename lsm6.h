#pragma once

class lsm6
{
public:
  static const int foo = 1;

  enum device_type
  {
    LSM6DS33 = 0x69,
  };

  enum i2c_addr
  {
    SA0_LOW_ADDR = 0x6A,
    SA0_HIGH_ADDR = 0x6B,
  };

  enum reg_addr
  {
    WHO_AM_I = 0x0F,
  };

private:
  i2c_bus i2c;
  i2c_addr address;
  device_type device;
};
