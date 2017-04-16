# Design goals

When I made `minimu9-ahrs`, I had several goals in mind.

First of all, this AHRS isn't just for model planes.  I wanted it to be
generally useful, so I wanted to avoid making any assumptions about what
orientation the IMU is in.  Therefore, I wanted to use an algorithm that treats
all three axes (X, Y, and Z) equally.  I looked at existing algorithms that
stored the current state as a Direction Cosine Matrix (DCM) and found that when
they normalized the DCM, they usually treated one axis specially.  For an
example of this problem, see the algorithm presented in the "Renormalization"
section of [Direction Cosine Matrix IMU Theory][1] by William Premerlani and
Paul Bizard.  I decided to use a quaternion instead of a DCM to store the
internal state because normalizing a quaternion is straightforward and treats
all axes equally.

Secondly, I wanted to have "gyro only" and "compass only" modes as a tool for
teaching people about how IMUs work and also to help troubleshoot systems where
one of those components might be acting incorrectly.  Therefore, I designed the
program to have different sensor fusion algorithms in it and you can easily pick
one at runtime with a command-line parameter.

Third, I wanted it to be easy to integrate the output of the program into a
larger system.  That's why I send the output as numbers to the standard output
pipe and provide several different output formats.

And finally, I wanted the code to have all the usual attributes that good code
has.  Short functions, small classes, and avoiding lots of magic numbers and
global variables are some of the things I had in mind.

[1]: http://gentlenav.googlecode.com/files/DCMDraft2.pdf