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

namespace minimu9
{
  // Represents the sensors of the MinIMU-9 and how to communicate with them.
  struct comm_config {
    lsm303::comm_config lsm303;
    l3g::comm_config l3g;
    lis3mdl::comm_config lis3mdl;
    lsm6::comm_config lsm6;
  };

  comm_config auto_detect(const std::string & i2c_bus);

  class handle : public imu {
  public:
    void open(const comm_config &);

    lsm303::handle compass;
    l3g::handle gyro;

    virtual vector read_acc();
    virtual vector read_mag();
    virtual vector read_gyro();

    virtual void enable();
    virtual void load_calibration();
    virtual void measure_offsets();
  };
}
