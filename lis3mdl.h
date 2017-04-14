#pragma once

#include <i2c_bus.h>
#include <cstdint>

class lis3mdl
{
public:
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
    WHO_AM_I = 0x0F,
  };

  struct comm_config {
    bool use_sensor = false;
    device_type device;
    std::string i2c_bus_name;
    i2c_addr i2c_address;
  };

  lis3mdl(const comm_config &);

private:
  i2c_bus i2c;
  i2c_addr address;
  device_type device;
};
