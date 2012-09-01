#ifndef _vector_h
#define _vector_h

#include <Eigen/Core>
#include <Eigen/Geometry>
typedef Eigen::Vector3f vector;
typedef Eigen::Vector3i int_vector;
typedef Eigen::Matrix3f matrix;
typedef Eigen::Quaternionf quaternion;

static inline vector vector_from_ints(int (*ints)[3])
{
    return vector((float)(*ints)[0], (float)(*ints)[1], (float)(*ints)[2]);
}

static inline int_vector int_vector_from_ints(int (*ints)[3])
{
    return int_vector((*ints)[0], (*ints)[1], (*ints)[2]);
}

#endif
