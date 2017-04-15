#ifndef _version_h
#define _version_h

#include <stdint.h>
#include <Eigen/Core>
#include <Eigen/Geometry>
typedef Eigen::Vector3f vector;
typedef Eigen::Vector3i int_vector;
typedef Eigen::Matrix3f matrix;
typedef Eigen::Quaternionf quaternion;

static inline vector vector_from_ints(int32_t (*ints)[3])
{
    return vector((float)(*ints)[0], (float)(*ints)[1], (float)(*ints)[2]);
}

#endif
