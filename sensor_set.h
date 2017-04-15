#pragma once

struct sensor_set
{
  bool mag = false;
  bool acc = false;
  bool gyro = false;

  operator bool()
  {
    return mag || acc || gyro;
  }

  inline friend sensor_set operator-(const sensor_set &, const sensor_set &);
};

inline sensor_set operator-(
  const sensor_set & c1,
  const sensor_set & c2)
{
  sensor_set r;
  r.mag = c1.mag && !c2.mag;
  r.acc = c1.acc && !c2.acc;
  r.gyro = c1.gyro && !c2.gyro;
  return r;
}
