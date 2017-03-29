# minimu9-ahrs

*Note: The versions of the MinIMU-9 that are supported by this program are all sold out.  I plan to add support for the MinIMU-9 v5 soon.  Until then, you can still get an [AltIMU-10 v4], which has the same chips as the [MinIMU-9 v3], so it will work with this program.*

**minimu9-ahrs** is a program for reading sensor data from [Pololu MinIMU-9] over I²C.  It supports MinIMU-9 versions v0, v1, v2, and v3.  The program can output the raw sensor data from the magnetometor, accelerometer, and gyro or it can use that raw data to compute the orientation of the IMU.  This program was designed and tested on the [Raspberry Pi], but it will probably work on any embedded Linux board that supports I²C.  A Debian package is available for easy installation and the [minimu9-ahrs source code] is on GitHub.

**For help getting started, please see: https://github.com/DavidEGrayson/minimu9-ahrs/wiki.**

## Building From Source

Navigate to the top-level directory of the source code and then run the following two commands:

<pre><code>sudo apt-get install libi2c-dev libeigen3-dev libboost-program-options-dev python-scipy
make</code></pre>

Then, to install minimu9-ahrs onto your system, run:

<pre><code>sudo make install</code></pre>

## Version History

- 2.0.0 (2014-07-08):
  - Added support for the MinIMU-9 v3 (LSM303D and L3GD20H)
  - Removed the right-shifting of raw accelerometer outputs; the raw readings are now signed 16-bit numbers that can range from -32768 to 32767.  Previously the readings were signed 12-bit numbers, so this new version effectively gives readings that are greater by a factor of 16.
  - Changed the minimu9-ahrs-calibrator Python script to use [SciPy] instead of the old optimization algorithm.
  - Changed the minimu9-ahrs-calibrator script to print a warning if there are fewer than 300 input vectors instead of exiting.
  - Changed the minimu9-ahrs-calibrator script to print a warning if the calibration looks wrong.
- 1.1.1 (2012-10-17)
- 1.1.0 (2012-10-15)
- 1.0.0 (2012-10-06)

[minimu9-ahrs source code]: https://github.com/DavidEGrayson/minimu9-ahrs
[Raspberry Pi]: http://www.raspberrypi.org
[AltIMU-10 v4]: https://www.pololu.com/product/2470
[MinIMU-9 v3]: https://www.pololu.com/product/2468
[Pololu MinIMU-9]:http://www.pololu.com/catalog/product/2468
[SciPy]:http://scipy.org
