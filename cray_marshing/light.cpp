#include "light.hpp"
#include <algorithm>

Light::Light() {}

Light::Light(const Vector origin, const Vector color_, std::string type_, double distance_coef_) {
    o = origin;
    color = color_;
    type = type_;
    distance_coef = distance_coef_;
}

Intersection test_ray(Ray &ray, const std::vector<Primitive*> &objects, const Primitive *to_ignore);

Vector Light::calculate_effect(const Vector &point, Vector &normal, const Primitive *obj, const std::vector<Primitive*> &objects) {
    if (type == POINT) {
        Vector p_o = o - point;
        if (p_o.len() == 0) {
            return color;
        }

        Ray ray(point + p_o.normal(), p_o.normal());

        //cout << obj << ' ' << objects[2] << endl;
        Intersection inter = test_ray(ray, objects, obj);
        double dist = inter.d;
        if (dist != -1 && dist < p_o.len() && !(inter.obj->props.refractive)) {
            return {0, 0, 0}; // maybe {AMBIENT, AMBIENT, AMBIENT} ???
        }

        Vector colorative_coef = Vector(1, 1, 1);
        if (dist != -1 && dist < p_o.len() && inter.obj->props.refractive) {
            colorative_coef += inter.obj->props.color;
            colorative_coef = colorative_coef / 2;
        }

        double reflection_coef = obj->props.refractive_coef;
        double intensity = distance_coef / pow(12.5 * p_o.len(), 2 - reflection_coef / 5); // BULLSHIT
        double power = std::max(normal.dot(p_o.normal()) * intensity, 0.0);
        if ((power - AMBIENT) > eps) {
            power += reflection_coef * pow(normal.dot(p_o.normal()), 100 * reflection_coef);
        }
        return color * power * colorative_coef;
    } else { // DISTANCE or trash
        Vector p_o = o.normal() * -1;
        Ray ray(point + p_o * eps, p_o);
        Intersection inter = test_ray(ray, objects, obj);
        if (inter.d != -1) {
            return Vector(0, 0, 0);
        } else {
            return color * (-o.dot(normal));
        }
    }
}

std::istream& operator>>(std::istream& input_stream, Light &light) {
    std::string type;
    Vector o, color;
    double distance_coef;
    input_stream >> type >> o >> color >> distance_coef;
    light = Light(o, color, type, distance_coef);
    return input_stream;
}
