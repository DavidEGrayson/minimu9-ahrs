#pragma once

#include "IMU.h"
#include "LSM303.h"
#include "L3G.h"

struct minimu9_sensor_config {
  bool has_sensor;
  unsigned int model_code;
  std::string i2c_bus;
  uint8_t i2c_address;
};

// Represents the sensors of the MinIMU-9 and how to communicate with them.
struct minimu9_config {
  minimu9_sensor_config lsm303, l3g, lis3mdl, lsm6;
};

minimu9_config minimu9_auto_detect(const std::string & i2c_bus);

class MinIMU9 : public IMU {
public:
  MinIMU9(const char * i2cDeviceName);

  LSM303 compass;
  L3G gyro;

  virtual vector readAcc();
  virtual vector readMag();
  virtual vector readGyro();

  virtual void enable();
  virtual void loadCalibration();
  virtual void measureOffsets();
};
