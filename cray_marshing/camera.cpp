#include "camera.hpp"

Camera::Camera() {}

Camera::Camera(const Vector origin, Vector direction, double distance, double width, double height, double res_coef) {
    o = origin;
    d = direction.normal();
    dist = distance;
    w = width;
    h = height;
    res_x = width * res_coef;
    res_y = height * res_coef;

    ort1 = Vector(-d.y, d.x, d.z).normal();
    ort2 = ort1.cross(d).normal();
    left_upper = o + d * dist + ort1 * w * 0.5 + ort2 * h * 0.5;
}

Ray Camera::get_ray(double x, double y) {
    Vector dx = ort1 * x * w / res_x;
    Vector dy = ort2 * y * h / res_y;
    return {o, (left_upper - dx - dy - o).normal()};
}

void Camera::update() {
    d = d.normal();
    //ort1 = Vector(-d.y, d.x, d.z).normal();
    double pi = 3.1415926535;
    Vector a(0, 0, -1);
    ort1 = a.cross(d).normal();
    ort2 = ort1.cross(d).normal();
    left_upper = o + d * dist + ort1 * w * 0.5 + ort2 * h * 0.5;
}
