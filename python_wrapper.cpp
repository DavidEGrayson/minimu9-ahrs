#include <boost/python.hpp>
#include "lsm6.h"
#include "lis3mdl.h"
#include <minimu9.h>
#include <string>
#include <vector>
#include <iostream>
using namespace boost::python;

namespace py = boost::python;

class minimu
{
	public:
	minimu9::handle imu;
	minimu9::comm_config config;
	minimu(std::string device){
		sensor_set set;
		set.mag = set.acc = set.gyro = true;
		config = minimu9::auto_detect(device);
		sensor_set missing = set - minimu9::config_sensor_set(config);

		if (missing)
		{
			if (missing.mag)
			{
			  std::cerr << "Error: No magnetometer found." << std::endl;
			}
			if (missing.acc)
			{
			  std::cerr << "Error: No accelerometer found." << std::endl;
			}
			if (missing.gyro)
			{
			  std::cerr << "Error: No gyro found." << std::endl;
			}
			std::cerr << "Error: Needed sensors are missing." << std::endl;
		}
		config = minimu9::disable_redundant_sensors(config, set);
		

	}
	void connect(){
		imu.open(config);
		imu.enable();
	}

	py::list read(){
		imu.read_raw();
		py::list l;
		l.append(imu.m[0]);
		l.append(imu.m[1]);
		l.append(imu.m[2]);
		l.append(imu.a[0]);
		l.append(imu.a[1]);
		l.append(imu.a[2]);
		l.append(imu.g[0]);
		l.append(imu.g[1]);
		l.append(imu.g[2]);
		return l;
	}
};

BOOST_PYTHON_MODULE(minimu)
{
	class_<minimu>("Minimu", init<std::string>())
		.def("read", &minimu::read)
		.def("connect", &minimu::connect)
	;
}