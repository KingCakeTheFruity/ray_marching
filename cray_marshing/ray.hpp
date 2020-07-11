#ifndef RAY
#define RAY

#include "vector.hpp"

struct Ray {
    Vector o;
    Vector d;

    Ray(const Vector &origin, const Vector &direction);
    Vector cast(double l);
};

#endif // RAY
