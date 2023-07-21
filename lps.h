#pragma once

#include <i2c_bus.h>
#include <cstdint>

namespace lps
{
  const uint8_t sa0_low_addr = 0x5C;

  enum device_type
  {
    LPS22DF = 0xB4,
  };

  enum reg_addr
  {
    CTRL_REG1 = 0x10,
    CTRL_REG2 = 0x11,
    CTRL_REG3 = 0x12,
    CTRL_REG4 = 0x13,
    PRESS_OUT_XL = 0x28,
  };

  struct comm_config {
    bool use_sensor = false;
    device_type device;
    std::string i2c_bus_name;
    uint8_t i2c_address;
  };

  class handle
  {
  public:
    void open(const comm_config &);

    void enable();

    void write_reg(reg_addr addr, uint8_t value);

    void read_pressure();

    int32_t pressure_raw;
    float pressure_inches_hg;

  protected:
    i2c_bus i2c;
    comm_config config;
  };
};
