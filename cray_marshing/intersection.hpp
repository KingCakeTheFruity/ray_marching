#ifndef INTERSECTION
#define INTERSECTION

#include "vector.hpp"

struct Primitive;

struct Intersection {
    Vector p;
    double d;
    Vector n;
    Primitive *obj;

    Intersection(const Vector point, double distance, const Vector normal, Primitive* object);
};

bool operator==(const Intersection &first, const Intersection &second);
bool operator<(const Intersection &first, const Intersection &second);

#endif // INTERSECTION
