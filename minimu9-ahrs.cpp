/*
promising:
http://elinux.org/Interfacing_with_I2C_Devices
https://i2c.wiki.kernel.org/index.php/Main_Page
*/

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include "LSM303.h"
#include "L3G4200D.h"

int main(int argc, char *argv[])
{
    I2CBus i2c("/dev/i2c-0");
    LSM303 compass(i2c);
    L3G4200D gyro(i2c);

    uint8_t result = compass.readMagReg(LSM303_WHO_AM_I_M);
    if (result != 0x3C)
    {
        std::cerr << "Error getting \"Who Am I\" register." << std::endl;
        exit(2);
    }

    compass.enableDefault();
    gyro.enableDefault();

    while(1)
    {
        compass.read();
        gyro.read();
        printf("%7d %7d %7d,  %7d %7d %7d,  %7d %7d %7d\n",
               compass.m(0), compass.m(1), compass.m(2),
               compass.a(0), compass.a(1), compass.a(2),
               gyro.g(0), gyro.g(1), gyro.g(2)
        );
        usleep(100*1000);
    }

    return 0;
}
