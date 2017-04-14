#pragma once

#include "i2c_bus.h"

class l3g
{
public:
  enum device_type
  {
    L3G4200D = 0xD3,
    L3GD20 = 0xD4,
    L3GD20H = 0xD7,
  };

  enum i2c_addr
  {
    L3GD20_SA0_LOW_ADDR = 0x6B,
    L3GD20_SA0_HIGH_ADDR = 0x6A,
    L3G4200D_SA0_LOW_ADDR = 0x68,
    L3G4200D_SA0_HIGH_ADDR = 0x69,
  };

  // For now, we use the old register names instead of the new ones that the
  // L3GD20H has.
  enum reg_addr
  {
    WHO_AM_I = 0x0F,
    CTRL_REG1 = 0x20,
    CTRL_REG2 = 0x21,
    CTRL_REG3 = 0x22,
    CTRL_REG4 = 0x23,
    CTRL_REG5 = 0x24,
    REFERENCE = 0x25,
    OUT_TEMP = 0x26,
    STATUS_REG = 0x27,
    OUT_X_L = 0x28,
    OUT_X_H = 0x29,
    OUT_Y_L = 0x2A,
    OUT_Y_H = 0x2B,
    OUT_Z_L = 0x2C,
    OUT_Z_H = 0x2D,
    FIFO_CTRL_REG = 0x2E,
    FIFO_SRC_REG = 0x2F,
    INT1_CFG = 0x30,
    INT1_SRC = 0x31,
    INT1_THS_XH = 0x32,
    INT1_THS_XL = 0x33,
    INT1_THS_YH = 0x34,
    INT1_THS_YL = 0x35,
    INT1_THS_ZH = 0x36,
    INT1_THS_ZL = 0x37,
    INT1_DURATION = 0x38,
    LOW_ODR = 0x39,
  };

  l3g(const std::string & i2c_bus);

  // gyro angular velocity readings
  int g[3];  // TODO: use int16_t, right?

  void enable();

  void writeReg(uint8_t reg, uint8_t value);
  uint8_t readReg(uint8_t reg);
  void read();

private:
  void detectAddress();
  i2c_bus i2c;
  i2c_addr address;
  device_type device;
};
