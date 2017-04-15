#include "vector.h"
#include "MinIMU9.h"
#include "exceptions.h"
#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <wordexp.h>

minimu9_comm_config minimu9_auto_detect(const std::string & i2c_bus_name)
{
  i2c_bus bus(i2c_bus_name.c_str());
  minimu9_comm_config config;

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

void MinIMU9::open(const minimu9_comm_config & config)
{
  // TODO: need to do some cool stuff here
  if (config.lsm6.use_sensor)
  {
    
  }
}

void MinIMU9::enable()
{
  compass.enable();
  gyro.enable();
}

void MinIMU9::load_calibration()
{
    wordexp_t expansion_result;
    wordexp("~/.minimu9-ahrs-cal", &expansion_result, 0);

    std::ifstream file(expansion_result.we_wordv[0]);
    if (file.fail())
    {
        throw posix_error("Failed to open calibration file ~/.minimu9-ahrs-cal");
    }

    file >> mag_min(0) >> mag_max(0) >> mag_min(1) >> mag_max(1) >> mag_min(2) >> mag_max(2);
    if (file.fail() || file.bad())
    {
        throw std::runtime_error("Failed to parse calibration file ~/.minimu9-ahrs-cal");
    }
}

void MinIMU9::measure_offsets()
{
    // LSM303 accelerometer: 8 g sensitivity.  3.8 mg/digit; 1 g = 256.
    // TODO: unify this with the other place in the code where we scale accelerometer readings.
    gyro_offset = vector::Zero();
    const int sampleCount = 32;
    for(int i = 0; i < sampleCount; i++)
    {
        gyro.read();
        gyro_offset += vector_from_ints(&gyro.g);
        usleep(20*1000);
    }
    gyro_offset /= sampleCount;
}

vector MinIMU9::read_mag()
{
    compass.read_mag();
    raw_m = int_vector_from_ints(&compass.m);

    vector v;
    v(0) = (float)(compass.m[0] - mag_min(0)) / (mag_max(0) - mag_min(0)) * 2 - 1;
    v(1) = (float)(compass.m[1] - mag_min(1)) / (mag_max(1) - mag_min(1)) * 2 - 1;
    v(2) = (float)(compass.m[2] - mag_min(2)) / (mag_max(2) - mag_min(2)) * 2 - 1;
    return v;
}

vector MinIMU9::read_acc()
{
    // Info about linear acceleration sensitivity from datasheets:
    // LSM303DLM: at FS = 8 g, 3.9 mg/digit (12-bit reading)
    // LSM303DLHC: at FS = 8 g, 4 mg/digit (12-bit reading probably an approximation)
    // LSM303DLH: at FS = 8 g, 3.9 mg/digit (12-bit reading)
    // LSM303D: at FS = 8 g, 0.244 mg/LSB (16-bit reading)
    const float accel_scale = 0.000244;

    compass.read_acc();
    imu::raw_a = int_vector_from_ints(&compass.a);
    return vector_from_ints(&compass.a) * accel_scale;
}

vector MinIMU9::read_gyro()
{
    // Info about sensitivity from datasheets:
    // L3G4200D: at FS = 2000 dps, 70 mdps/digit
    // L3GD20: at FS = 2000 dps, 70 mdps/digit
    // L3GD20H: at FS = 2000 dps, 70 mdps/digit
    const float gyro_scale = 0.07 * 3.14159265 / 180;

    gyro.read();
    raw_g = int_vector_from_ints(&gyro.g);
    return ( vector_from_ints(&gyro.g) - gyro_offset ) * gyro_scale;
}
