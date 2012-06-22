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
#include "LSM303.h"

int main()
{
    const char * devName = "/dev/i2c-0";

    // Open up the I2C bus
    int fd = open(devName, O_RDWR);
    if (fd == -1)
    {
        perror(devName);
        exit(1);
    }

    LSM303 lsm303(fd);
    uint8_t result = lsm303.readMagReg(LSM303_WHO_AM_I_M);
    printf("result: 0x%02X\n", result);

    return 0;
}
