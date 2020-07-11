#include "properties.hpp"

Properties::Properties() {}

Properties::Properties(const Vector color_, double reflective_, double refractive_, double refractive_coef_,
                       double type_, double scale_, const Vector rotation_, bool constant_color_) {
    color = color_;
    reflective = reflective_;
    refractive = refractive_;
    refractive_coef = refractive_coef_;
    type = type_;
    scale = scale_;
    rotation = rotation_;
    constant_color = constant_color_;
}

std::istream& operator>>(std::istream& input_stream,  Properties &prop) {
    char bracket;
    input_stream >> bracket;
    Vector color, rotation;
    double reflective, refractive, refractive_coef, type, scale, constant_color;
    input_stream >> color >> reflective >> refractive >> refractive_coef >> type >> scale >> rotation >> constant_color;
    input_stream >> bracket;
    prop = Properties(color, reflective, refractive, refractive_coef, type, scale, rotation, constant_color);

    return input_stream;
}
