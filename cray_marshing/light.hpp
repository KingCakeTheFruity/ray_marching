#ifndef LIGHT
#define LIGHT

#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include "vector.hpp"
#include "primitive.hpp"
#include "intersection.hpp"

extern const double AMBIENT;

struct Light {
    Vector o;
    Vector color;
    std::string type;
    double distance_coef;

    Light();
    Light(const Vector origin, const Vector color_, std::string type_, double distance_coef_);
    Vector calculate_effect(const Vector &point, Vector &normal, const Primitive *obj, const std::vector<Primitive*> &objects);
};

std::istream& operator>>(std::istream& input_stream, Light &light);

#endif // LIGHT
