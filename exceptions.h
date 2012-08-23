#ifndef _AHRS_EXCEPTIONS_H
#define _AHRS_EXCEPTIONS_H

#include <stdexcept>

class PosixError : std::runtime_error
{
    virtual const char* what() const throw();
};

#endif
