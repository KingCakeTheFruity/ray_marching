#include "vector.hpp"
#include <cmath>

using namespace std;

const double eps = 1e-5;

Vector::Vector() {
    x = 0.0;
    y = 0.0;
    z = 0.0;
}

Vector::Vector(double x_, double y_, double z_) {
    x = x_;
    y = y_;
    z = z_;
}

double Vector::len() const{
    return sqrt(x * x + y * y + z * z);
}

Vector Vector::normal() const{
    double l = len();
    if (l < eps) {
        return {0, 0, 0};
    } else {
        return {x / l, y / l, z / l};
    }
}

double Vector::dot(const Vector &other) const{
    return x * other.x + y * other.y + z * other.z;
}

Vector Vector::cross(const Vector &other) const{
    double res_x = y * other.z - z * other.y;
    double res_y = z * other.x - x * other.z;
    double res_z = x * other.y - y * other.x;
    return {res_x, res_y, res_z};
}

Vector operator+(const Vector &first, const Vector &second) {
    return {first.x + second.x, first.y + second.y, first.z + second.z};
}

Vector operator+=(Vector &first, const Vector &second) {
    first.x += second.x;
    first.y += second.y;
    first.z += second.z;
    return first;
}

Vector operator-(const Vector &first, const Vector &second) {
    return {first.x - second.x, first.y - second.y, first.z - second.z};
}

Vector operator-=(Vector& first, const Vector& second) {
    first.x -= second.x;
    first.y -= second.y;
    first.z -= second.z;
    return first;
}

Vector operator*(const Vector &first, const Vector &second) {
    return {first.x * second.x, first.y * second.y, first.z * second.z};
}

Vector operator*=(Vector &first, const Vector &second) {
    first.x *= second.x;
    first.y *= second.y;
    first.z *= second.z;
    return first;
}

Vector operator*(const Vector &first, const double k) {
    return {first.x * k, first.y * k, first.z * k};
}

Vector operator*=(Vector &first, const double k) {
    first.x *= k;
    first.y *= k;
    first.z *= k;
    return first;
}

Vector operator/(const Vector &first, const Vector &second) {
    return {first.x / second.x, first.y / second.y, first.z / second.z};
}

Vector operator/(const Vector &first, const double k) {
    return {first.x / k, first.y / k, first.z / k};
}

bool operator==(const Vector &first, const Vector &second) {
    return (first.x - second.x) < eps  && (first.y - second.y) < eps && (first.z -second.z) < eps;
}

bool operator<(const Vector &first, const Vector &second) {
    return first.x < second.x  && first.y < second.y && first.z < second.z;
}

std::ostream& operator<<(std::ostream& output_stream, const Vector& vector) {
    output_stream << vector.x << ' ' << vector.y << ' ' << vector.z;
    return output_stream;
}

istream& operator>>(istream& input_stream, Vector &vec) {
    char coma, bracket;
    double x, y, z;
    input_stream >> bracket >> x >> coma >> y >> coma >> z >> bracket;
    vec.x = x;
    vec.y = y;
    vec.z = z;

    return input_stream;
}

Vector oriented(Vector axis, Vector other) {
    if (axis.dot(other) < 0) {
        other = other * (-1.0);
    }
    return other;
}

Vector rotx(const Vector vec, double ang) {
    double x = vec.x;
    double y = vec.y * cos(ang) - vec.z * sin(ang);
    double z = vec.y * sin(ang) + vec.z * cos(ang);
    return {x, y, z};
}

Vector roty(const Vector vec, double ang) {
    double x = vec.x * cos(ang) + vec.z * sin(ang);
    double y = vec.y;
    double z = -vec.x * sin(ang) + vec.z * cos(ang);
    return {x, y, z};
}

Vector rotz(const Vector vec, double ang) {
    double x = vec.x * cos(ang) - vec.y * sin(ang);
    double y = vec.x * sin(ang) + vec.y * cos(ang);
    double z = vec.z;
    return {x, y, z};
}

Vector rotate(const Vector vec, double dx, double dy, double dz) {
    return rotz(roty(rotx(vec, dx), dy), dz);
}

Vector rotate(const Vector vec, const Vector rotation) {
    return rotz(roty(rotx(vec, rotation.x), rotation.y), rotation.z);
}

Line::Line() {
    p = {0, 0, 0};
    d = {0, 0, 0};
}

Line::Line(const Vector &p1, const Vector &p2) {
    p = p1;
    d = (p2 - p1).normal();
}

Vector Line::point_projection(const Vector &point) {
    double t = (point - p).dot(d) / d.len();
    return p + d * t;
}

Vector Line::clamped_point_projection(const Vector &p2, const Vector &point) {
    Vector proj = point_projection(point);
    Vector p2_p = proj - p2;
    Vector p1_p = proj - p;
    if (p2_p.dot(p1_p) > eps) {
        if (p2_p.len() < p1_p.len()) {
            return p2;
        } else {
            return p;
        }
    } else {
        return proj;
    }
}
