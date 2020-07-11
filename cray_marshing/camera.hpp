#ifndef CAMERA
#define CAMERA

#include "vector.hpp"
#include "ray.hpp"

struct Camera {
    Vector o;
    Vector d;
    double dist;
    double w;
    double h;
    int res_x;
    int res_y;
    Vector ort1;
    Vector ort2;
    Vector left_upper;

    Camera ();
    Camera(const Vector origin, Vector direction, double distance, double width, double height, double res_coef);
    Ray get_ray(double x, double y);
    void update();
};

#endif // CAMERA
