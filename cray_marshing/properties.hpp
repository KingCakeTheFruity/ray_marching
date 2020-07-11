#ifndef PROPERTIES
#define PROPERTIES

#include <iostream>
#include "vector.hpp"

struct Properties {
    Vector color;
    double reflective;
    double refractive;
    double refractive_coef;
    double type;
    double scale;
    Vector rotation;
    bool constant_color;

    Properties();
    Properties(const Vector color_, double reflective_, double refractive_, double refractive_coef_, double type_,
               double scale_, const Vector rotation_, bool constant_color_);
};

std::istream& operator>>(std::istream& input_stream,  Properties &prop);

#endif // PROPERTIES
