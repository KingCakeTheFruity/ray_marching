#include "primitive.hpp"
#include <iostream>

const int FILL = 2;
const int SQUARED = 3;
const int DISTANT = 4;
const int SPHERE = 6;
const int PLANE = 7;
const int POLYGON = 8;
const std::string POINT = "point";

Intersection Primitive::intersect(Ray &ray) {return {{0, 0, 0}, -1, Vector(0, 0, 0), this};}

Sphere::Sphere() {}

Sphere::Sphere(const Vector center, double radius, const Properties &properties) {
    type = SPHERE;
    c = center;
    r = radius;
    props = properties;
}

Vector Sphere::normal(const Vector &point) {
    return (point - c).normal();
}

Intersection Sphere::intersect(Ray &ray) {
    Vector c_o = ray.o - c;
    double disc = r * r - (c_o.dot(c_o) - ray.d.dot(c_o) * ray.d.dot(c_o));
    if (disc < 0) {
        return {{0, 0, 0}, -1, Vector(0, 0, 0), this};
    }

    double b = -ray.d.dot(c_o);
    double d1 = b - sqrt(disc);
    double d2 = b + sqrt(disc);
    double d = -1;
    if (d1 > 0 && (d2 > d1 || d2 < 0)) {
        d = d1;
    } else if (d2 > 0 && (d1 > d2 || d1 < 0)) {
        d = d2;
    } else {
        return {{0, 0, 0}, -1, Vector(0, 0, 0), this};
    }

    Vector point = ray.cast(d);
    return {point, d, this->normal(point), this};
}

Vector Sphere::intersects(Primitive *prim) {
    if (prim->type == SPHERE) {
        Sphere* sphere = dynamic_cast<Sphere*>(prim);
        double centers_distant = (sphere->c - c).len();
        if (centers_distant - (sphere->r + r) < eps && (centers_distant - abs(sphere->r - r) > eps)) {
            return (c - sphere->c).normal() * (-1) * dist(sphere);
        }
    } else if (prim->type == POLYGON) {
        Triangle* triag = dynamic_cast<Triangle*>(prim);
        Vector p = triag->get_nearest_point(c);
        if ((p - c).len() - r < eps) {
            if ((triag->p1 - c).len() - r > eps || (triag->p1 - c).len() - r > eps || (triag->p1 - c).len() - r > eps) {
                Vector proj = triag->pl.point_projection(c);
                Vector proj_c = c - proj;
                return proj_c.normal() * (r - proj_c.len());
            }
        }
    } else if (prim->type == PLANE) {
        Plane* pl = dynamic_cast<Plane*>(prim);
        Vector proj = pl->point_projection(c);
        Vector proj_c = c - proj;
        if (proj_c.len() - r < eps) {
            return proj_c.normal() * (r - proj_c.len());
        }
    }
    return {0, 0, 0};
}

double Sphere::dist(Primitive* prim) const {
    if (prim->type == SPHERE) {
        Sphere* sphere = dynamic_cast<Sphere*>(prim);
        return (sphere->c - c).len() - (sphere->r + r);
    }
    else if (prim->type == POLYGON) {
        Triangle* triag = dynamic_cast<Triangle*>(prim);
        Vector p = triag->get_nearest_point(c);
        return (p - c).len() - r;
    }
    else if (prim->type == PLANE) {
        Plane* pl = dynamic_cast<Plane*>(prim);
        Vector proj = pl->point_projection(c);
        return (proj - c).len() - r;
    }
    return INFINITY;
}

Plane::Plane() {}

Plane::Plane(const Vector point, const Vector normal, const Properties &properties) {
    type = PLANE;
    p = point;
    n = normal;
    props = properties;
}

Intersection Plane::intersect(Ray &ray) {
    double cs = n.dot(ray.d);
    if (fabs(cs) < eps || cs > 0) {
        return {{0, 0, 0}, -1, Vector(0, 0, 0), this};
    }
    double d = (p -  ray.o).dot(n) / cs;

    Vector point = ray.cast(d);
    return {point, d, n, this};
}

Vector Plane::point_projection(const Vector& point) {
    Vector p_point = point - p;
    double t = n.dot(p_point) / n.len();
    return point - n.normal() * t;
}

Triangle::Triangle() {}

Triangle::Triangle(const Vector point_1, const Vector point_2, const Vector point_3, const Properties &properties) {
    type = POLYGON;
    p1 = point_1;
    p2 = point_2;
    p3 = point_3;
    n = (p2 - p1).cross(p3 - p1).normal();
    pl = Plane(p1, n, properties);
    sq = (p2 - p1).cross(p3 - p1).len() / 2;
    props = properties;
}

bool Triangle::is_point_inside(const Vector &p) {
    if (fabs((p - p1).dot(n)) > eps) {
        return false;
    }
    Vector p_p1 = p1 - p;
    Vector p_p2 = p2 - p;
    Vector p_p3 = p3 - p;
    double square = (p_p1.cross(p_p2).len() + p_p2.cross(p_p3).len() + p_p3.cross(p_p1).len()) / 2;
    return !(fabs(sq - square) > eps);
}

Intersection Triangle::intersect(Ray &ray) {
    auto orig = pl.n;
    pl.n = oriented(ray.d * (-1), pl.n);
    Intersection inter = pl.intersect(ray);
    pl.n = orig;
    if (inter.d == -1 || !is_point_inside(inter.p) || ray.d.dot(inter.p - ray.o) < 0) {
        return {{0, 0, 0}, -1, Vector(0, 0, 0), this};
    } else {
        inter.obj = this;
        return inter;
    }
}

Vector Triangle::get_nearest_point(const Vector &p) {
    Vector proj = pl.point_projection(p);
    if (is_point_inside(proj)) {
        return proj;
    }

    Vector proj_12 = Line(p1, p2).clamped_point_projection(p2, p);
    Vector proj_23 = Line(p2, p3).clamped_point_projection(p3, p);
    Vector proj_31 = Line(p3, p1).clamped_point_projection(p1, p);

    Vector proj_p_1 = p - proj_12;
    Vector proj_p_2 = p - proj_23;
    Vector proj_p_3 = p - proj_31;

    double min_dist = proj_p_1.len();
    Vector ret = proj_12;
    if (proj_p_2.len() < min_dist) {
        min_dist = proj_p_2.len();
        ret = proj_23;
    }
    if (proj_p_3.len() < min_dist) {
        min_dist = proj_p_3.len();
        ret = proj_31;
    }
    return ret;
}

std::istream& operator>>(std::istream& input_stream, Primitive* &prim) {
    std::string type;
    input_stream >> type;
    if (type == "Sphere") {
        Sphere *obj = new Sphere;
        input_stream >> *obj;
        prim = obj;
    } else if (type == "Plane") {
        Plane *obj = new Plane;
        input_stream >> *obj;
        prim = obj;
    } else if (type == "Triangle") {
        Triangle *obj = new Triangle;
        input_stream >> *obj;
        prim = obj;
    }

    return input_stream;
}

std::istream& operator>>(std::istream& input_stream, Sphere &sph) {
    Vector c;
    double r;
    Properties props;
    input_stream >> c >> r >> props;
    sph = Sphere(c, r, props);
    return input_stream;
}

std::istream& operator>>(std::istream& input_stream, Plane &pl) {
    Vector p, n;
    Properties props;
    input_stream >> p >> n >> props;
    pl = Plane(p, n, props);
    return input_stream;
}

std::istream& operator>>(std::istream& input_stream, Triangle &triag) {
    Vector p1, p2, p3;
    Properties props;
    input_stream >> p1 >> p2 >> p3 >> props;
    triag = Triangle(p1, p2, p3, props);
    return input_stream;
}
