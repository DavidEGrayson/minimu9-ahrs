#include "prog_options.h"
#include <iostream>
#include <boost/program_options.hpp>
namespace opts = boost::program_options;

static opts::options_description options_desc(prog_options & options)
{
  // Define what all the command-line parameters are.
  opts::options_description desc("Allowed options");
  desc.add_options()
    ("help,h",
      "produce help message")
    ("version,v",
      "print version number")
    ("i2c-bus,b",
      opts::value<std::string>(&options.i2c_bus_name)->default_value("/dev/i2c-0"),
     "i2c-bus the IMU is connected to")
    ("mode",
      opts::value<std::string>(&options.mode)->default_value("normal"),
     "specifies what algorithm to use.\n"
     "normal: Fuse compass and gyro.\n"
     "gyro-only:  Use only gyro (drifts).\n"
     "compass-only:  Use only compass (noisy).\n"
     "raw: Just print raw values from sensors.")
    ("output",
      opts::value<std::string>(&options.output_mode)->default_value("matrix"),
     "specifies how to output the orientation.\n"
     "matrix: Direction Cosine Matrix.\n"
     "quaternion: Quaternion.\n"
     "euler: Euler angles (yaw, pitch, roll).\n")
    ;
  return desc;
}

void print_command_line_options_desc()
{
  prog_options options;
  std::cout << options_desc(options) << std::endl;
}

prog_options get_prog_options(int argc, char ** argv)
{
  prog_options options;
  opts::options_description desc = options_desc(options);
  opts::variables_map vmap;
  opts::store(opts::command_line_parser(argc, argv).options(desc).run(), vmap);
  opts::notify(vmap);
  if (vmap.count("help")) { options.show_help = true; }
  if (vmap.count("version")) { options.show_version = true; }

  return options;
}
