#include "prog_options.h"
#include <iostream>
#include <boost/program_options.hpp>
namespace opts = boost::program_options;

static opts::options_description general_options_desc(prog_options & options)
{
  opts::options_description desc("General options");
  desc.add_options()
    ("help,h",
      "produce help message")
    ("version,v",
      "print version number")
    ;
  return desc;
}

static opts::options_description sensor_options_desc(prog_options & options)
{
  opts::options_description desc("Sensor options");
  desc.add_options()
    ("i2c-bus,b",
      opts::value<std::string>(&options.i2c_bus_name)->default_value("/dev/i2c-0"),
     "I2C bus the IMU is connected to")
    ;
  return desc;
}

static opts::options_description processing_options_desc(prog_options & options)
{
  opts::options_description desc("Processing options");
  desc.add_options()
    ("mode",
      opts::value<std::string>(&options.mode)->default_value("normal"),
      "specifies what algorithm to use.\n"
      "normal: Fuse compass and gyro.\n"
      "gyro-only:  Use only gyro (drifts).\n"
      "compass-only:  Use only compass (noisy).\n"
      // TODO: "scaled:  Just print scaled values from sensor axes.\n")
      "raw: Just print raw values from sensors axes.")
    ("output",
      opts::value<std::string>(&options.output_mode)->default_value("matrix"),
      "specifies how to output the orientation.\n"
      "matrix: Direction Cosine Matrix.\n"
      "quaternion: Quaternion.\n"
      "euler: Euler angles (yaw, pitch, roll).\n")
    ;
  return desc;
}

static opts::options_description command_line_options_desc(prog_options & options)
{
  return general_options_desc(options)
    .add(sensor_options_desc(options))
    .add(processing_options_desc(options))
    ;
}

void print_command_line_options_desc()
{
  prog_options options;
  std::cout << command_line_options_desc(options) << std::endl;
}

prog_options get_prog_options(int argc, char ** argv)
{
  prog_options options;

  // TODO: reject positional args

  auto desc = command_line_options_desc(options);
  auto parser = opts::command_line_parser(argc, argv).options(desc);
  opts::variables_map vmap;
  opts::store(parser.run(), vmap);
  opts::notify(vmap);
  if (vmap.count("help")) { options.show_help = true; }
  if (vmap.count("version")) { options.show_version = true; }

  return options;
}

// TODO: I guess this part should handle calibration files too
