#pragma once

#include "IMU.h"
#include "lsm303.h"
#include "l3g.h"
#include "lsm6.h"
#include "lis3mdl.h"

struct minimu9_sensor_config {
  bool use_sensor = false;
  uint32_t device_type = 0;
  std::string i2c_bus_name;
  uint8_t i2c_address = 0;
};

struct minimu9_lsm303_config {
  bool use_sensor = false;
  uint32_t device_type = 0;
  std::string i2c_bus_name;
  uint8_t i2c_address_acc = 0;
  uint8_t i2c_address_mag = 0;
};

// Represents the sensors of the MinIMU-9 and how to communicate with them.
struct minimu9_config {
  minimu9_lsm303_config lsm303;
  minimu9_sensor_config l3g, lis3mdl, lsm6;
};

minimu9_config minimu9_auto_detect(const std::string & i2c_bus);

class MinIMU9 : public IMU {
public:
  MinIMU9(const char * i2cDeviceName);

  lsm303 compass;
  l3g gyro;

  virtual vector readAcc();
  virtual vector readMag();
  virtual vector readGyro();

  virtual void enable();
  virtual void loadCalibration();
  virtual void measureOffsets();
};
