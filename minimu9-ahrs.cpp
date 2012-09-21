#include "vector.h"
#include "MinIMU9.h"
#include "version.h"
#include <iostream>
#include <iomanip>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <system_error>
#include <boost/program_options.hpp>

namespace opts = boost::program_options;

// TODO: print warning if accelerometer magnitude is not close to 1 when starting up

#define FLOAT_FORMAT std::fixed << std::setprecision(4) << std::setw(7)

std::ostream & operator << (std::ostream & os, const vector & vector)
{
    return os << FLOAT_FORMAT << vector(0) << ' '
              << FLOAT_FORMAT << vector(1) << ' '
              << FLOAT_FORMAT << vector(2);
}

std::ostream & operator << (std::ostream & os, const matrix & matrix)
{
    return os << (vector)matrix.row(0) << ' '
              << (vector)matrix.row(1) << ' '
              << (vector)matrix.row(2);
}

std::ostream & operator << (std::ostream & os, const quaternion & quat)
{
    return os << FLOAT_FORMAT << quat.w() << ' '
              << FLOAT_FORMAT << quat.x() << ' '
              << FLOAT_FORMAT << quat.y() << ' '
              << FLOAT_FORMAT << quat.z();
}

int millis()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec) * 1000 + (tv.tv_usec)/1000;
}

void streamRawValues(IMU& imu)
{
    imu.enable();
    while(1)
    {
        imu.read();
        printf("%7d %7d %7d  %7d %7d %7d  %7d %7d %7d\n",
               imu.raw_m[0], imu.raw_m[1], imu.raw_m[2],
               imu.raw_a[0], imu.raw_a[1], imu.raw_a[2],
               imu.raw_g[0], imu.raw_g[1], imu.raw_g[2]
        );
        usleep(20*1000);
    }
}

matrix rotationFromCompass(const vector& acceleration, const vector& magnetic_field)
{
    vector up = acceleration;     // usually true
    vector east = magnetic_field.cross(up); // actual it's magnetic east
    vector north = up.cross(east);

    matrix rotationFromCompass;
    rotationFromCompass.row(0) = east;
    rotationFromCompass.row(1) = north;
    rotationFromCompass.row(2) = up;
    rotationFromCompass.row(0).normalize();
    rotationFromCompass.row(1).normalize();
    rotationFromCompass.row(2).normalize();

    return rotationFromCompass;
}

float heading(matrix rotation)
{
    // The board's x axis in earth coordinates.
    vector x = rotation.col(0);
    x.normalize();
    x(2) = 0;

    // 0 = east, pi/2 = north
    return atan2(x(1), x(0));
}

typedef void fuse_function(quaternion& rotation, float dt, const vector& angular_velocity,
                  const vector& acceleration, const vector& magnetic_field);

typedef void rotation_output_function(quaternion& rotation);

void output_matrix(quaternion & rotation)
{
    std::cout << rotation.toRotationMatrix();
}

void output_quaternion(quaternion & rotation)
{
    std::cout << rotation;
}

void output_euler(quaternion & rotation)
{
    throw std::runtime_error("Euler angle output not implemented yet"); // TODO
}

void fuse_compass_only(quaternion& rotation, float dt, const vector& angular_velocity,
  const vector& acceleration, const vector& magnetic_field)
{
    // Implicit conversion of rotation matrix to quaternion.
    rotation = rotationFromCompass(acceleration, magnetic_field);
}

// w is angular velocity in radiands per second.
// dt is the time.
void rotate(quaternion& rotation, const vector& w, float dt)
{
    // First order approximation of the quaternion representing this rotation.
    quaternion q = quaternion(1, w(0)*dt/2, w(1)*dt/2, w(2)*dt/2);
    rotation *= q;
    rotation.normalize();
}

void fuse_gyro_only(quaternion& rotation, float dt, const vector& angular_velocity,
  const vector& acceleration, const vector& magnetic_field)
{
    rotate(rotation, angular_velocity, dt);
}

void fuse_default(quaternion& rotation, float dt, const vector& angular_velocity,
  const vector& acceleration, const vector& magnetic_field)
{
    vector correction = vector(0, 0, 0);

    if (abs(acceleration.norm() - 1) <= 0.3)
    {
        // The magnetidude of acceleration is close to 1 g, so
        // it might be pointing up and we can do drift correction.

        const float correction_strength = 1;

        matrix rotationCompass = rotationFromCompass(acceleration, magnetic_field);
        matrix rotationMatrix = rotation.toRotationMatrix();

        correction = (
            rotationCompass.row(0).cross(rotationMatrix.row(0)) +
            rotationCompass.row(1).cross(rotationMatrix.row(1)) +
            rotationCompass.row(2).cross(rotationMatrix.row(2))
          ) * correction_strength;

    }

    rotate(rotation, angular_velocity + correction, dt);
}

void ahrs(IMU & imu, fuse_function * fuse, rotation_output_function * output)
{
    imu.loadCalibration();
    imu.enable();
    imu.measureOffsets();
    
    // The quaternion that can convert a vector in body coordinates
    // to ground coordinates when it its changed to a matrix.
    quaternion rotation = quaternion::Identity();

    int start = millis(); // truncate 64-bit return value
    while(1)
    {
        int last_start = start;
        start = millis();
        float dt = (start-last_start)/1000.0;
        if (dt < 0){ throw std::runtime_error("Time went backwards."); }

        vector angular_velocity = imu.readGyro();
        vector acceleration = imu.readAcc();
        vector magnetic_field = imu.readMag();

        fuse(rotation, dt, angular_velocity, acceleration, magnetic_field);

        output(rotation);
        std::cout << "  " << acceleration << "  " << magnetic_field << std::endl << std::flush;

        // Ensure that each iteration of the loop takes at least 20 ms.
        while(millis() - start < 20)
        {
            usleep(1000);
        }
    }
}

std::pair<std::string, std::string> option_translator(const std::string& s)
{
    if (s == "--gyro-only")
    {
        return std::make_pair("mode", "gyro-only");
    }
    else if (s == "--compass-only")
    {
        return std::make_pair("mode", "compass-only");
    }
    else if (s == "--raw")
    {
        return std::make_pair("mode", "raw");
    }
    else
    {
        return std::make_pair(std::string(), std::string());
    }
}

int main(int argc, char *argv[])
{
    try
    {
        // Define what all the command-line parameters are.
        std::string mode, output_mode;
        opts::options_description desc("Allowed options");
        desc.add_options()
            ("help,h", "produce help message")
            ("version,v", "print version number")
            ("mode", opts::value<std::string>(&mode)->default_value("normal"),
             "normal: Fuse compass and gyro.\n"
             "gyro-only:  Use only gyro (drifts).\n"
             "compass-only:  Use only compass (noisy).\n"
             "raw: Just print raw values from sensors.")
            ("output", opts::value<std::string>(&output_mode)->default_value("matrix"),
             "matrix: Direction Cosine Matrix.\n"
             "quaternion: Quaternion.\n"
             "euler: Euler angle (yaw, pitch, roll).\n")
            ;
        opts::variables_map options;
        opts::store(opts::command_line_parser(argc, argv).options(desc).extra_parser(option_translator).run(), options);
        opts::notify(options);

        if(options.count("help"))
        {
            std::cout << desc << std::endl;
            return 0;
        }
        
        if (options.count("version"))
        {
            std::cout << VERSION << std::endl;
            return 0;
        }

        MinIMU9 imu("/dev/i2c-0");
        
        imu.checkConnection();

        rotation_output_function * output;

        // Figure out the output mode.
        if (output_mode == "matrix")
        {
            output = &output_matrix;
        }
        else if (output_mode == "quaternion")
        {
            output = &output_quaternion;
        }
        else if (output_mode == "euler")
        {
            output = &output_euler;
        }
        else
        {
            std::cerr << "Unknown output mode '" << output_mode << "'" << std::endl;
            return 1;
        }

        // Figure out the basic operating mode and start running.
        if (mode == "raw")
        {
            streamRawValues(imu);
        }
        else if (mode == "gyro-only")
        {
            ahrs(imu, &fuse_gyro_only, output);
        }
        else if (mode == "compass-only")
        {
            ahrs(imu, &fuse_compass_only, output);
        }
        else if (mode == "normal")
        {
            ahrs(imu, &fuse_default, output);
        }
        else
        {
            std::cerr << "Unknown mode '" << mode << "'" << std::endl;
            return 1;
        }
        return 0;
    }
    catch(const std::system_error & error)
    {
        std::string what = error.what();
        const std::error_code & code = error.code();
        std::cerr << "Error: " << what << "  " << code.message() << " (" << code << ")" << std::endl;
        return 2;
    }
    catch(const opts::multiple_occurrences & error)
    {
        std::cerr << "Error: " << error.what() << " of " << error.get_option_name() << " option." << std::endl;
        return 1;
    }
    catch(const std::exception & error)    
    {
        std::cerr << "Error: " << error.what() << std::endl;
        return 9;
    }
}
