#include <iostream>

/*
promising:
http://elinux.org/Interfacing_with_I2C_Devices
https://i2c.wiki.kernel.org/index.php/Main_Page
*/

#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

// maybe it should be 0x3C, not shifted
#define MAG_ADDRESS            (0x3C >> 1)
#define LSM303_CTRL_REG1_A      0x20

int main()
{
    const char * devName = "/dev/i2c-0";
    char buf[10];
    int file = open(devName, O_RDWR);
    if (file == -1)
    {
        perror(devName);
        exit(1);
    }

    if (ioctl(file, I2C_SLAVE, MAG_ADDRESS) < 0) {
        perror("Failed to acquire bus access and/or talk to slave");
        exit(1);
    }

    buf[0] = LSM303_CTRL_REG1_A; 
    if (write(file, buf, 1) != 1) {
        perror("Failed to write to the i2c bus");
    }

    // I2C read
    if (read(file,buf,1) != 1) {
        perror("Failed to read from the i2c bus");
        exit(1);
    }

    printf("result: %02x\n", buf[0]);

    return 0;
}
