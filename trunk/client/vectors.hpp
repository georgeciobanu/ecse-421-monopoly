#ifndef VECTORS_HPP
#define VECTORS_HPP

#define MATH_VECTOR_NO_IMPLICIT_CONVERSION
#define MATH_VECTOR_AGGREGATE
#include "math_vector.hpp"

typedef math::vector<2, float> vector2f;
typedef math::vector<3, unsigned char> vector3ub;

#define RET_VEC_2F(a,b) do { vector2f p = {{(a),(b)}}; return p; } while (0)

bool inside( vector2f const &x,
             vector2f const &lt,
             vector2f const &br ) {

    return lt.x() <= x.x() && x.x() <= br.x()
        && lt.y() >= x.y() && x.y() >= br.y();
}

#endif

