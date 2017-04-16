# minimu9-ahrs

**minimu9-ahrs** is a program for reading sensor data from the Pololu MinIMU-9
and similar boards over I²C.  It supports the following sensor chips:

* LSM6DS33 accelerometer and gyro
* LIS3MDL magnetometer
* LSM303D magnetometer and accelerometer
* LSM303DLHC magnetometer and accelerometer
* LSM303DLM magnetometer and accelerometer
* LSM303DLH magnetometer and accelerometer
* L3GD20H gyro
* L3GD20 gyro
* L3G4200D gyro

This program works with any board that has a magnetometer, acceleromter, and
a gyro from the list above.  This includes the following Pololu products:

* MinIMU-9 [v0][mv0], [v1][mv1], [v2][mv2], [v3][mv3], [v5][mv5]
* AltIMU-10 [v3][av3], [v4][av4], [v5][av5]
* [Balboa 32U4 Balancing Robot Kit][balboa]

The program can output the raw sensor data from the magnetometor, accelerometer,
and gyro or it can use that raw data to compute the orientation of the IMU.
This program was designed and tested on the [Raspberry Pi], but it will probably
work on any embedded Linux board that supports I²C.  A Debian package is
available for easy installation and the [minimu9-ahrs source code] is on GitHub.

**For help getting started, please see: https://github.com/DavidEGrayson/minimu9-ahrs/wiki.**

## Building From Source

Navigate to the top-level directory of the source code and then run the following two commands:

<pre><code>sudo apt-get install libi2c-dev libeigen3-dev libboost-program-options-dev python-scipy
make</code></pre>

Then, to install minimu9-ahrs onto your system, run:

<pre><code>sudo make install</code></pre>

## Visualization

See the [ahrs-visualizer project] if you are interesting in displaying the output of minimu9-ahrs in 3D.

## Version History

- 3.0.0 (2017-04-15):
  - Added support for the MinIMU-9 v5 (LSM6DS33 and LIS3MDL).
  - Added support for a configuration file at `~/.minimu9-ahrs`.
  - Fixed a bug that resulted in corrections from the accelerometer always being applied
    even if the acceleration magnitude was not close to 1 g (thanks nxdefiant).
  - Use a Linux timerfd for more accurate timing (thanks to nxdefiant for the idea).
  - Made the `minimu9-ahrs-calibrate` script store the raw data in `~/.minimu9-ahrs-cal-data`.
  - Changed the minimu9-ahrs-calibrator Python script to just do simple minima/maxima because the fancy optimization algorithm was not reliable.
  - Fixed the way boost_program_options is linked.
- 2.0.0 (2014-07-08):
  - Added support for the MinIMU-9 v3 (LSM303D and L3GD20H)
  - Removed the right-shifting of raw accelerometer outputs; the raw readings are now signed 16-bit numbers that can range from -32768 to 32767.  Previously the readings were signed 12-bit numbers, so this new version effectively gives readings that are greater by a factor of 16.
  - Changed the minimu9-ahrs-calibrator Python script to use SciPy instead of the old optimization algorithm.
  - Changed the minimu9-ahrs-calibrator script to print a warning if there are fewer than 300 input vectors instead of exiting.
  - Changed the minimu9-ahrs-calibrator script to print a warning if the calibration looks wrong.
- 1.1.1 (2012-10-17)
- 1.1.0 (2012-10-15)
- 1.0.0 (2012-10-06)

[ahrs-visualizer project]: https://github.com/DavidEGrayson/ahrs-visualizer
[minimu9-ahrs source code]: https://github.com/DavidEGrayson/minimu9-ahrs
[Raspberry Pi]: https://www.raspberrypi.org
[av3]: https://www.pololu.com/product/2469
[av4]: https://www.pololu.com/product/2470
[av5]: https://www.pololu.com/product/2739
[mv0]: https://www.pololu.com/product/1264
[mv1]: https://www.pololu.com/product/1265
[mv2]: https://www.pololu.com/product/1268
[mv3]: https://www.pololu.com/product/2468
[mv5]: https://www.pololu.com/product/2738
[balboa]: https://www.pololu.com/product/3575

