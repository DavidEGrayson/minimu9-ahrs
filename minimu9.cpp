#include "vector.h"
#include "minimu9.h"
#include "exceptions.h"
#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <wordexp.h>

minimu9::comm_config minimu9::auto_detect(const std::string & i2c_bus_name)
{
  i2c_bus bus(i2c_bus_name.c_str());
  minimu9::comm_config config;

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
    auto addrs = { lis3mdl::SA1_LOW_ADDR, lis3mdl::SA1_LOW_ADDR };
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

  // Detect L3G devices.
  {
    auto addrs = {
      l3g::L3GD20_SA0_LOW_ADDR,
      l3g::L3GD20_SA0_HIGH_ADDR,
      l3g::L3G4200D_SA0_LOW_ADDR,
      l3g::L3G4200D_SA0_HIGH_ADDR,
    };
    for (uint8_t addr : addrs)
    {
      int result = bus.try_write_byte_and_read_byte(addr, l3g::WHO_AM_I);
      if (result == l3g::L3G4200D || result == l3g::L3GD20
        || result == l3g::L3GD20H)
      {
        config.l3g.use_sensor = true;
        config.l3g.device = (l3g::device_type)result;
        config.l3g.i2c_bus_name = i2c_bus_name;
        config.l3g.i2c_address = (l3g::i2c_addr)addr;
        break;
      }
    }
  }

  // Detect LSM303 devices.
  {
    auto & c = config.lsm303;
    if (lsm303::LSM303D == bus.try_write_byte_and_read_byte(
        lsm303::LSM303D_SA0_HIGH_ADDR, lsm303::WHO_AM_I))
    {
      c.use_sensor = true;
      c.device = lsm303::LSM303D;
      c.i2c_bus_name = i2c_bus_name;
      c.i2c_address_acc = c.i2c_address_mag =
        lsm303::LSM303D_SA0_HIGH_ADDR;
    }
    else if (lsm303::LSM303D == bus.try_write_byte_and_read_byte(
        lsm303::LSM303D_SA0_LOW_ADDR, lsm303::WHO_AM_I))
    {
      c.use_sensor = true;
      c.device = lsm303::LSM303D;
      c.i2c_bus_name = i2c_bus_name;
      c.i2c_address_acc = c.i2c_address_mag =
        lsm303::LSM303D_SA0_LOW_ADDR;
    }
    // Remaining possibilities: LSM303DLHC, LSM303DLM, or LSM303DLH.
    //
    // LSM303DLHC seems to respond to WHO_AM_I request the same way as DLM, even
    // though this register isn't documented in its datasheet, and LSM303DLH does
    // not have a WHO_AM_I.
    //
    // Lets assume that if it's a LSM303DLM and LSM303DLH then SA0 is low,
    // because that is how the Pololu boards pull that pin and this lets us
    // distinguish between the LSM303DLHC and those other chips.
    else if (bus.try_write_byte_and_read_byte(
        lsm303::LSM303_NON_D_ACC_SA0_HIGH_ADDR, lsm303::CTRL_REG1_A) >= 0)
    {
      // There's an accelerometer on a chip with SA0 high, so by the
      // logic above, guess that it's an LSM303DLHC.
      c.use_sensor = true;
      c.device = lsm303::LSM303DLHC;
      c.i2c_bus_name = i2c_bus_name;
      c.i2c_address_acc = lsm303::LSM303_NON_D_ACC_SA0_HIGH_ADDR;
      c.i2c_address_mag = lsm303::LSM303_NON_D_MAG_ADDR;
    }
    // Remaining possibilities: LSM303DLM or LSM303DLH, SA0 assumed low.
    else if (bus.try_write_byte_and_read_byte(
        lsm303::LSM303_NON_D_ACC_SA0_LOW_ADDR, lsm303::CTRL_REG1_A) >= 0)
    {
      // Found the acceleromter for an LSM303DLM or LSM303DLH.
      // Use the WHO_AM_I register to distinguish the two.

      int result = bus.try_write_byte_and_read_byte(
        lsm303::LSM303_NON_D_MAG_ADDR, lsm303::WHO_AM_I_M);

      if (result == lsm303::LSM303DLM)
      {
        // Detected LSM303DLM with SA0 low.
        c.use_sensor = true;
        c.device = lsm303::LSM303DLM;
        c.i2c_bus_name = i2c_bus_name;
        c.i2c_address_acc = lsm303::LSM303_NON_D_ACC_SA0_LOW_ADDR;
        c.i2c_address_mag = lsm303::LSM303_NON_D_MAG_ADDR;
      }
      else
      {
        // Guess that it's an LSM303DLH with SA0 low.
        c.use_sensor = true;
        c.device = lsm303::LSM303DLH;
        c.i2c_bus_name = i2c_bus_name;
        c.i2c_address_acc = lsm303::LSM303_NON_D_ACC_SA0_LOW_ADDR;
        c.i2c_address_mag = lsm303::LSM303_NON_D_MAG_ADDR;
      }
    }
  }

  return config;
}

sensor_set minimu9::config_sensor_set(const comm_config & config)
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

  if (config.lsm303.use_sensor)
  {
    set.mag = true;
    set.acc = true;
  }

  if (config.l3g.use_sensor)
  {
    set.gyro = true;
  }

  return set;
}

minimu9::comm_config minimu9::disable_redundant_sensors(
  const comm_config & in, const sensor_set & needed)
{
  comm_config config = in;

  sensor_set missing = needed;

  if (!(missing.acc || missing.gyro))
  {
    config.lsm6.use_sensor = false;
  }
  else if (config.lsm6.use_sensor)
  {
    missing.acc = false;
    missing.gyro = false;
  }

  if (!missing.mag)
  {
    config.lis3mdl.use_sensor = false;
  }
  else if (config.lis3mdl.use_sensor)
  {
    missing.mag = false;
  }

  if (!(missing.mag || missing.acc))
  {
    config.lsm303.use_sensor = false;
  }
  else if (config.lsm303.use_sensor)
  {
    missing.mag = false;
    missing.acc = false;
  }

  if (!missing.gyro)
  {
    config.l3g.use_sensor = false;
  }
  else if (config.l3g.use_sensor)
  {
    missing.gyro = false;
  }

  return config;
}

void minimu9::handle::open(const comm_config & config)
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

  if (config.lsm303.use_sensor)
  {
    lsm303.open(config.lsm303);
  }

  if (config.l3g.use_sensor)
  {
    l3g.open(config.l3g);
  }
}

void minimu9::handle::enable()
{
  if (config.lsm6.use_sensor)
  {
    lsm6.enable();
  }

  if (config.lis3mdl.use_sensor)
  {
    lis3mdl.enable();
  }

  if (config.lsm303.use_sensor)
  {
    lsm303.enable();
  }

  if (config.l3g.use_sensor)
  {
    l3g.enable();
  }
}

void minimu9::handle::load_calibration()
{
  wordexp_t expansion_result;
  wordexp("~/.minimu9-ahrs-cal", &expansion_result, 0);

  std::ifstream file(expansion_result.we_wordv[0]);
  if (file.fail())
  {
    throw posix_error("Failed to open calibration file ~/.minimu9-ahrs-cal");
  }

  file >> mag_min(0) >> mag_max(0)
       >> mag_min(1) >> mag_max(1)
       >> mag_min(2) >> mag_max(2);
  if (file.fail() || file.bad())
  {
    throw std::runtime_error("Failed to parse calibration file ~/.minimu9-ahrs-cal");
  }
}

void minimu9::handle::read_mag_raw()
{
  if (config.lis3mdl.use_sensor)
  {
    lis3mdl.read();
    for (int i = 0; i < 3; i++) { m[i] = lis3mdl.m[i]; }
  }
  else if (config.lsm303.use_sensor)
  {
    lsm303.read_mag();
    for (int i = 0; i < 3; i++) { m[i] = lsm303.m[i]; }
  }
  else
  {
    throw std::runtime_error("No magnetometer to read.");
  }
}

void minimu9::handle::read_acc_raw()
{
  if (config.lsm6.use_sensor)
  {
    lsm6.read_acc();
    for (int i = 0; i < 3; i++) { a[i] = lsm6.a[i]; }
  }
  else if (config.lsm303.use_sensor)
  {
    lsm303.read_acc();
    for (int i = 0; i < 3; i++) { a[i] = lsm303.a[i]; }
  }
  else
  {
    throw std::runtime_error("No accelerometer to read.");
  }
}

void minimu9::handle::read_gyro_raw()
{
  if (config.lsm6.use_sensor)
  {
    lsm6.read_gyro();
    for (int i = 0; i < 3; i++) { g[i] = lsm6.g[i]; }
  }
  else if (config.l3g.use_sensor)
  {
    l3g.read();
    for (int i = 0; i < 3; i++) { g[i] = l3g.g[i]; }
  }
  else
  {
    throw std::runtime_error("No gyro to read.");
  }
}

float minimu9::handle::get_acc_scale() const
{
  // Info about linear acceleration sensitivity from datasheets:
  // LSM303DLM: at FS = 8 g, 3.9 mg/digit (12-bit reading)
  // LSM303DLHC: at FS = 8 g, 4 mg/digit (12-bit reading probably an approximation)
  // LSM303DLH: at FS = 8 g, 3.9 mg/digit (12-bit reading)
  // LSM303D: at FS = 8 g, 0.244 mg/LSB (16-bit reading)
  // LSM6DS33: at FS = 8 g, 0.244 mg/LSB (16-bit reading)
  return 0.000244;
}

float minimu9::handle::get_gyro_scale() const
{
  // Info about sensitivity from datasheets:
  // L3G4200D, FS = 2000 dps: 70 mdps/digit
  // L3GD20,   FS = 2000 dps: 70 mdps/digit
  // L3GD20H,  FS = 2000 dps: 70 mdps/digit
  // LSM6DS33, FS = 2000 dps: 70 mdps/digit
  return 0.07 * 3.14159265 / 180;
}

void minimu9::handle::measure_offsets()
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

vector minimu9::handle::read_mag()
{
  read_mag_raw();

  vector v;
  v(0) = (float)(m[0] - mag_min(0)) / (mag_max(0) - mag_min(0)) * 2 - 1;
  v(1) = (float)(m[1] - mag_min(1)) / (mag_max(1) - mag_min(1)) * 2 - 1;
  v(2) = (float)(m[2] - mag_min(2)) / (mag_max(2) - mag_min(2)) * 2 - 1;
  return v;
}

vector minimu9::handle::read_acc()
{
  read_acc_raw();
  return vector_from_ints(&a) * get_acc_scale();
}

vector minimu9::handle::read_gyro()
{
  read_gyro_raw();
  return (vector_from_ints(&g) - gyro_offset) * get_gyro_scale();
}
