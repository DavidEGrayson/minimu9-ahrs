#!/bin/bash
# Run as root.

sudo modprobe i2c-dev

# maybe it should be 0x1E, not sure what the convention is
# echo lsm303mag 0x3C > /sys/class/i2c-adapter/i2c-0/new_device
