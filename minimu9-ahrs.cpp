//#include <iostream>

/*
promising:
http://elinux.org/Interfacing_with_I2C_Devices
https://i2c.wiki.kernel.org/index.php/Main_Page
*/

#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include "i2c-dev.h"

#define MAG_ADDRESS        (0x3C >> 1)
#define LSM303_WHO_AM_I_M  (0x0F)

int main()
{
    const char * devName = "/dev/i2c-0";

    // Open up the I2C bus
    int file = open(devName, O_RDWR);
    if (file == -1)
    {
        perror(devName);
        exit(1);
    }

    // Specify the address of the slave device.
    if (ioctl(file, I2C_SLAVE, MAG_ADDRESS) < 0)
    {
        perror("Failed to acquire bus access and/or talk to slave");
        exit(1);
    }

    int result = i2c_smbus_read_byte_data(file, LSM303_WHO_AM_I_M);
    printf("result: 0x%02X\n", result);

    return 0;
}
