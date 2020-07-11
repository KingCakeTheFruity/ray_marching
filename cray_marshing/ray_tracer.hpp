#ifndef RAY_TRACER
#define RAY_TRACER

#include <iostream>
#include <string>
#include <vector>
#include "vector.hpp"
#include "ray.hpp"
#include "primitive.hpp"
#include "model.hpp"
#include "light.hpp"
#include "camera.hpp"
#include "scene.hpp"

using namespace std;

extern const double inf;

extern const double AMBIENT;
extern const Vector BACKGROUND;

template <typename T>
T sign(T x);

double g(double x, double y, double z);

Intersection test_ray(Ray &ray, const vector<Primitive*> &objects, const Primitive *to_ignore);
Vector trace_ray(Ray &ray, const vector<Primitive*> &objects, const vector<Light> &lights, int depth);

vector<vector<Vector>> render_image(Camera camera, vector<Primitive*> objects, vector<Light> lights, int depth, int verbose);
vector<int> get_color(const Vector &color);
void save_image(vector<vector<Vector>> &image, string filename);

#endif // RAY_TRACER
