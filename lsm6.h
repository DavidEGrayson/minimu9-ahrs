#pragma once

#include <i2c_bus.h>
#include <cstdint>

namespace lsm6
{
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

    // acceleration readings
    int16_t a[3];

    // gyro angular velocity readings
    int16_t g[3];

  protected:
    i2c_bus i2c;
    comm_config config;
  };
};
