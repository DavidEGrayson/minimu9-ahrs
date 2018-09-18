#include "vector.h"
#include "altimu10.h"
#include "exceptions.h"
#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <wordexp.h>

altimu10::comm_config altimu10::auto_detect(const std::string & i2c_bus_name)
{
  i2c_bus bus(i2c_bus_name.c_str());
  altimu10::comm_config config;

  // Detect LSM6 devices.
  {
    auto addrs = { lsm6::SA0_LOW_ADDR, lsm6::SA0_HIGH_ADDR };
    for (uint8_t addr : addrs)
    {
      int result = bus.try_write_byte_and_read_byte(addr, lsm6::WHO_AM_I);
      if (result == lsm6::LSM6DS33)
      {
        config.lsm6.use_sensor = true;
        config.lsm6.device = (lsm6::device_type)result;
        config.lsm6.i2c_bus_name = i2c_bus_name;
        config.lsm6.i2c_address = (lsm6::i2c_addr)addr;
        break;
      }
    }
  }

  // Detect LIS3MDL devices.
  {
    auto addrs = { lis3mdl::SA1_LOW_ADDR, lis3mdl::SA1_HIGH_ADDR };
    for (uint8_t addr : addrs)
    {
      int result = bus.try_write_byte_and_read_byte(addr, lis3mdl::WHO_AM_I);
      if (result == lis3mdl::LIS3MDL)
      {
        config.lis3mdl.use_sensor = true;
        config.lis3mdl.device = (lis3mdl::device_type)result;
        config.lis3mdl.i2c_bus_name = i2c_bus_name;
        config.lis3mdl.i2c_address = (lis3mdl::i2c_addr)addr;
        break;
      }
    }
  }


  // Detect LPS25H devices.
  {
    auto & c = config.lps25h;
    if (lps25h::LPS25H == bus.try_write_byte_and_read_byte(
        lps25h::SA0_HIGH_ADDR, lps25h::WHO_AM_I))
    {
      c.use_sensor = true;
      c.device = lps25h::LPS25H;
      c.i2c_bus_name = i2c_bus_name;
      c.i2c_address = lps25h::SA0_HIGH_ADDR;
    }

  }

  return config;
}

sensor_set altimu10::config_sensor_set(const comm_config & config)
{
  sensor_set set;

  if (config.lsm6.use_sensor)
  {
    set.acc = true;
    set.gyro = true;
  }

  if (config.lis3mdl.use_sensor)
  {
    set.mag = true;
  }

  if (config.lps25h.use_sensor)
  {
    set.temp = true;
    set.press = true;
  }

  return set;
}

altimu10::comm_config altimu10::disable_redundant_sensors(
  const comm_config & in, const sensor_set & needed)
{
  comm_config config = in;

  return config;
}

void altimu10::handle::open(const comm_config & config)
{
  this->config = config;

  if (config.lsm6.use_sensor)
  {
    lsm6.open(config.lsm6);
  }

  if (config.lis3mdl.use_sensor)
  {
    lis3mdl.open(config.lis3mdl);
  }

  if (config.lps25h.use_sensor)
  {
    lps25h.open(config.lps25h);
  }
}

void altimu10::handle::enable()
{
  if (config.lsm6.use_sensor)
  {
    lsm6.enable();
  }

  if (config.lis3mdl.use_sensor)
  {
    lis3mdl.enable();
  }

  if (config.lps25h.use_sensor)
  {
    lps25h.enable();
  }
}

void altimu10::handle::load_calibration()
{
  wordexp_t expansion_result;
  wordexp("~/.minimu9-ahrs-cal", &expansion_result, 0);

  std::ifstream file(expansion_result.we_wordv[0]);
  if (file.fail())
  {
    throw posix_error("Failed to open calibration file ~/.ahrs-cal");
  }

  file >> mag_min(0) >> mag_max(0)
       >> mag_min(1) >> mag_max(1)
       >> mag_min(2) >> mag_max(2);
  if (file.fail() || file.bad())
  {
    throw std::runtime_error("Failed to parse calibration file ~/.ahrs-cal");
  }
}

void altimu10::handle::read_mag_raw()
{
  if (config.lis3mdl.use_sensor)
  {
    lis3mdl.read();
    for (int i = 0; i < 3; i++) { m[i] = lis3mdl.m[i]; }
  }
  else
  {
    throw std::runtime_error("No magnetometer to read.");
  }
}

void altimu10::handle::read_acc_raw()
{
  if (config.lsm6.use_sensor)
  {
    lsm6.read_acc();
    for (int i = 0; i < 3; i++) { a[i] = lsm6.a[i]; }
  }
  else
  {
    throw std::runtime_error("No accelerometer to read.");
  }
}

void altimu10::handle::read_gyro_raw()
{
  if (config.lsm6.use_sensor)
  {
    lsm6.read_gyro();
    for (int i = 0; i < 3; i++) { g[i] = lsm6.g[i]; }
  }
  else
  {
    throw std::runtime_error("No gyro to read.");
  }
}

void altimu10::handle::read_press_raw()
{
  if (config.lps25h.use_sensor)
  {
    lps25h.read_press();
    p = lps25h.p;
    refp = lps25h.refp;
  }
  else
  {
    throw std::runtime_error("No barometer to read.");
  }
}

void altimu10::handle::read_temp_raw()
{
  if (config.lps25h.use_sensor)
  {
    lps25h.read_temp();
    t = lps25h.t;
  }
  else
  {
    throw std::runtime_error("No termometer found.");
  }
}

float altimu10::handle::get_acc_scale() const
{
  // Info about linear acceleration sensitivity from datasheets:
  // LSM303DLM: at FS = 8 g, 3.9 mg/digit (12-bit reading)
  // LSM303DLHC: at FS = 8 g, 4 mg/digit (12-bit reading probably an approximation)
  // LSM303DLH: at FS = 8 g, 3.9 mg/digit (12-bit reading)
  // LSM303D: at FS = 8 g, 0.244 mg/LSB (16-bit reading)
  // LSM6DS33: at FS = 8 g, 0.244 mg/LSB (16-bit reading)
  return 0.000244;
}

float altimu10::handle::get_gyro_scale() const
{
  // Info about sensitivity from datasheets:
  // L3G4200D, FS = 2000 dps: 70 mdps/digit
  // L3GD20,   FS = 2000 dps: 70 mdps/digit
  // L3GD20H,  FS = 2000 dps: 70 mdps/digit
  // LSM6DS33, FS = 2000 dps: 70 mdps/digit
  return 0.07 * 3.14159265 / 180;
}

void altimu10::handle::measure_offsets()
{
  // LSM303 accelerometer: 8 g sensitivity.  3.8 mg/digit; 1 g = 256.
  gyro_offset = vector::Zero();
  const int sampleCount = 32;
  for(int i = 0; i < sampleCount; i++)
  {
    read_gyro_raw();
    gyro_offset += vector_from_ints(&g);
    usleep(20 * 1000);
  }
  gyro_offset /= sampleCount;
}

vector altimu10::handle::read_mag()
{
  read_mag_raw();

  vector v;
  v(0) = (float)(m[0] - mag_min(0)) / (mag_max(0) - mag_min(0)) * 2 - 1;
  v(1) = (float)(m[1] - mag_min(1)) / (mag_max(1) - mag_min(1)) * 2 - 1;
  v(2) = (float)(m[2] - mag_min(2)) / (mag_max(2) - mag_min(2)) * 2 - 1;
  return v;
}

vector altimu10::handle::read_acc()
{
  read_acc_raw();
  return vector_from_ints(&a) * get_acc_scale();
}

vector altimu10::handle::read_gyro()
{
  read_gyro_raw();
  return (vector_from_ints(&g) - gyro_offset) * get_gyro_scale();
}

float altimu10::handle::read_temp()
{
  read_temp_raw();
  return (((float)t/480) + 42.5);
}

float altimu10::handle::read_press()
{
  read_press_raw();
  return ((float)(p+refp)/4096.f);
}
