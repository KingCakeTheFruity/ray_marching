#ifndef PRIMITIVE
#define PRIMITIVE

#include <iostream>
#include <string>
#include <cmath>
#include "properties.hpp"
#include "ray.hpp"
#include "intersection.hpp"

extern const int FILL;
extern const int SQUARED;
extern const int DISTANT;
extern const int SPHERE;
extern const int PLANE;
extern const int POLYGON;
extern const std::string POINT;


struct Primitive {
    Properties props;
    int type;
    virtual Intersection intersect(Ray &ray);
};

struct Sphere : Primitive {
    Vector c;
    double r;

    Sphere();
    Sphere(const Vector center, double radius, const Properties &properties);

    Vector normal(const Vector &point);
    Intersection intersect(Ray &ray);
    Vector intersects(Primitive *prim);
    double dist(Primitive *prim) const;
};

struct Plane : Primitive {
    Vector p;
    Vector n;

    Plane();
    Plane(const Vector point, const Vector normal, const Properties &properties);

    Intersection intersect(Ray &ray);
    Vector point_projection(const Vector &point);
};

struct Triangle : Primitive {
    Vector p1;
    Vector p2;
    Vector p3;
    Vector n;
    Plane pl;
    double sq;

    Triangle();
    Triangle(const Vector point_1, const Vector point_2, const Vector point_3, const Properties &properties);

    bool is_point_inside(const Vector &p);
    Intersection intersect(Ray &ray);
    Vector get_nearest_point(const Vector &p);
};

std::istream& operator>>(std::istream& input_stream, Primitive* &prim);
std::istream& operator>>(std::istream& input_stream, Sphere &sph);
std::istream& operator>>(std::istream& input_stream, Plane &pl);
std::istream& operator>>(std::istream& input_stream, Triangle &triag);

#endif // PRIMITIVE
