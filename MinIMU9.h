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

// Represents the sensors of the MinIMU-9 and how to communicate with them.
struct minimu9_comm_config {
  lsm303::comm_config lsm303_config;
  l3g::comm_config l3g_config;
  lis3mdl::comm_config lis3mdl_config;
  lsm6::comm_config lsm6_config;
};

minimu9_comm_config minimu9_auto_detect(const std::string & i2c_bus);

class MinIMU9 : public IMU {
public:
  void open(const minimu9_comm_config &);

  lsm303 compass;
  l3g gyro;

  virtual vector readAcc();
  virtual vector readMag();
  virtual vector readGyro();

  virtual void enable();
  virtual void loadCalibration();
  virtual void measureOffsets();
};
