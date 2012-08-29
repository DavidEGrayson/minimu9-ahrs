#ifndef _AHRS_EXCEPTIONS_H
#define _AHRS_EXCEPTIONS_H

#include <cerrno>
#include <system_error>

static inline std::system_error posix_error()
{
    return std::system_error(std::error_code(errno, std::system_category()));
}

#endif
