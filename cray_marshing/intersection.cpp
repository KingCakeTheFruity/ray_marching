#include "intersection.hpp"

Intersection::Intersection(const Vector point, double distance, const Vector normal, Primitive* object) {
    p = point;
    d = distance;
    n = normal;
    obj = object;
}

bool operator==(const Intersection &first, const Intersection &second) {
    return first.p == second.p && first.d == second.d && first.n == second.n && first.obj == second.obj;
}

bool operator<(const Intersection &first, const Intersection &second) {
    if (first.d == -1) {
        return false;
    } else if (second.d == -1) {
        return true;
    } else {
        return first.d < second.d;
    }
}
