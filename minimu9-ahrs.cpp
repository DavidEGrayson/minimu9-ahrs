/*
promising:
http://elinux.org/Interfacing_with_I2C_Devices
https://i2c.wiki.kernel.org/index.php/Main_Page
*/

#include <iostream>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include "LSM303.h"

int main(int argc, char *argv[])
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
    if (result != 0x3C)
    {
        std::cerr << "Error getting \"Who Am I\" register." << std::endl;
        exit(2);
    }

    while(1)
    {
        lsm303.read();
        std::cout.width(6);
        std::cout << std::right << lsm303.m << std::endl;
        usleep(300*1000);
    }

    return 0;
}
