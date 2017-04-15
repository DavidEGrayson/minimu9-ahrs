#pragma once

#include "vector.h"

class imu
{
public:
  virtual void read_raw()
  {
    read_mag_raw();
    read_acc_raw();
    read_gyro_raw();
  }

  virtual void read_acc_raw() = 0;
  virtual void read_mag_raw() = 0;
  virtual void read_gyro_raw() = 0;

  virtual float get_acc_scale() const = 0;
  virtual float get_gyro_scale() const = 0;

  int32_t m[3];
  int32_t a[3];
  int32_t g[3];

  // TODO: remove stuff below this point

  // Scaled readings
  virtual vector read_mag() = 0;  // In body coords, scaled to -1..1 range
  virtual vector read_acc() = 0;  // In body coords, with units = g
  virtual vector read_gyro() = 0; // In body coords, with units = rad/sec
  void read(){ read_mag(); read_acc(); read_gyro(); }

  virtual void measure_offsets() = 0;
  virtual void enable() = 0;
  virtual void load_calibration() = 0;

  vector gyro_offset;
  int_vector mag_min, mag_max;
};
