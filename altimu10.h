#pragma once

// TODO: I'd rather have the minimu9 and imu class not know anything about
// floating point vectors and calibration.  Maybe just provide floating
// point scaling factors so that higher-level code can make sense of the
// raw vectors.

#include "imu.h"
#include "lsm303.h"
#include "l3g.h"
#include "lsm6.h"
#include "lis3mdl.h"
#include "lps25h.h"
#include "sensor_set.h"

namespace altimu10
{
  // Represents the sensors of the AltIMU-10v5 and how to communicate with them.
  struct comm_config {
    lis3mdl::comm_config lis3mdl;
    lsm6::comm_config lsm6;
    lps25h::comm_config lps25h;
  };

  comm_config auto_detect(const std::string & i2c_bus_name);

  sensor_set config_sensor_set(const comm_config &);

  comm_config disable_redundant_sensors(const comm_config &, const sensor_set &);

  class handle : public imu {
  public:
    void open(const comm_config &);

    comm_config config;
    lsm6::handle lsm6;
    lis3mdl::handle lis3mdl;
    lps25h::handle lps25h;

    virtual void read_acc_raw();
    virtual void read_mag_raw();
    virtual void read_gyro_raw();
    virtual void read_temp_raw();
    virtual void read_press_raw();

    virtual float get_acc_scale() const;
    virtual float get_gyro_scale() const;

    virtual vector read_acc();
    virtual vector read_mag();
    virtual vector read_gyro();
    virtual uint32_t read_temp();
    virtual uint32_t read_press();

    virtual void enable();
    virtual void load_calibration();
    virtual void measure_offsets();
  };
}
