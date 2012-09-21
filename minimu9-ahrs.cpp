#include "vector.h"
#include "MinIMU9.h"
#include "version.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <system_error>
#include <boost/program_options.hpp>

namespace opts = boost::program_options;

// TODO: print warning if accelerometer magnitude is not close to 1 when starting up

int millis()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec) * 1000 + (tv.tv_usec)/1000;
}

void print(matrix m)
{
    printf("%7.4f %7.4f %7.4f %7.4f %7.4f %7.4f %7.4f %7.4f %7.4f",
           m(0,0), m(0,1), m(0,2),
           m(1,0), m(1,1), m(1,2),
           m(2,0), m(2,1), m(2,2));
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

std::ostream& operator << (std::ostream& os, const matrix& mc)
{
    os << "woohoo! ";
    return os;
}


void ahrs(IMU& imu, fuse_function * fuse_func)
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
        if (dt < 0){ throw std::runtime_error("time went backwards"); }

        vector angular_velocity = imu.readGyro();
        vector acceleration = imu.readAcc();
        vector magnetic_field = imu.readMag();

        fuse_func(rotation, dt, angular_velocity, acceleration, magnetic_field);

        matrix r = rotation.toRotationMatrix();
        std::cout << r;
        printf("%7.4f %7.4f %7.4f %7.4f %7.4f %7.4f %7.4f %7.4f %7.4f  %7.4f %7.4f %7.4f  %7.4f %7.4f %7.4f\n",
               r(0,0), r(0,1), r(0,2),
               r(1,0), r(1,1), r(1,2),
               r(2,0), r(2,1), r(2,2),
               acceleration(0), acceleration(1), acceleration(2),
               magnetic_field(0), magnetic_field(1), magnetic_field(2));
        fflush(stdout);

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
        std::string mode;
        opts::options_description desc("Allowed options");
        desc.add_options()
            ("help,h", "produce help message")
            ("version,v", "print version number")
            ("mode", opts::value<std::string>(&mode)->default_value("normal"),
             "normal (default): Fuse compass and gyro.\n"
             "gyro-only:  Use only gyro (drifts).\n"
             "compass-only:  Use only compass (noisy).\n"
             "raw: Just print raw values from sensors.")
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

        if (mode == "raw")
        {
            streamRawValues(imu);
        }
        else if (mode == "gyro-only")
        {
            ahrs(imu, &fuse_gyro_only);
        }
        else if (mode == "compass-only")
        {
            ahrs(imu, &fuse_compass_only);
        }
        else if (mode == "normal")
        {
            ahrs(imu, &fuse_default);
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
