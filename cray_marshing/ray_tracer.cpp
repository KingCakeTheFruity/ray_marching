#include "ray_tracer.hpp"
#include <cmath>
#include <fstream>
#include <random>

using namespace std;

const double inf = 1e10;

const double AMBIENT = 0.1;
const Vector BACKGROUND {AMBIENT, AMBIENT, AMBIENT};

std::default_random_engine gen;
std::uniform_real_distribution<double> uniform(-1.0, 1.0);

template <typename T>
T sign(T x) {
    if (x > 0) {
        return 1;
    } else if (x < 0) {
        return -1;
    } else {
        return 0;
    }
}

double g(double x, double y, double z) {
    return sin(x) * sin(y) * sin(z);
}


int cnttt = 0;

Intersection test_ray(Ray &ray, const vector<Primitive*> &objects, const Primitive *to_ignore) {
    Intersection inter = {{0, 0, 0}, -1, {0, 0, 0}, nullptr};
    for (int i = 0; (size_t)i < objects.size(); ++i) {
        Primitive* obj = objects[i];
        //if (to_ignore) cout << obj << ' ' << to_ignore << endl;
        if (obj == to_ignore) {
            continue;
        }

        Intersection cur_inter = obj->intersect(ray);
        if (ray.d.dot(cur_inter.p - ray.o) > 0) {
            inter = min(inter, cur_inter);
            //cout << ray.o << " | " << ray.d << endl;
        }
    }
    return inter;
}

vector<int> get_color(const Vector &color) {
    vector<int> ret(3, 0);
    ret[0] = min(255, (int)color.x);
    ret[1] = min(255, (int)color.y);
    ret[2] = min(255, (int)color.z);
    return ret;
}

Vector trace_ray(Ray &ray, const vector<Primitive*> &objects, const vector<Light> &lights, int depth) {
    if (!depth) {
        return BACKGROUND;
    }
    Intersection inter = test_ray(ray, objects, nullptr);
    //cout << ray.o << " | " << ray.d << " | " << inter.p << '\n';
    if (inter.d == -1) {
        return BACKGROUND;
    }

    Vector point = inter.p;
    Primitive obj = *inter.obj;
    Vector color = obj.props.color;
    if (obj.props.constant_color) {
        return color;
    }
    Vector light_effect;
    Vector reflected_color;
    Vector refracted_color;

    if (obj.props.type == SQUARED) { // Scale squares
        double x = sign(sin(point.x / obj.props.scale));
        double y = sign(sin(point.y / obj.props.scale));
        double z = sign(sin(point.z / obj.props.scale));
        color *= sign(g(x, y, z));
    }

    if (false) {
        double x = uniform(gen);
        double y = uniform(gen);
        double z = uniform(gen);
        Vector vec(x, y, z);
        vec = oriented(inter.n, vec);
        Ray reflected_ray(point + vec * eps, vec);
        Vector le = trace_ray(reflected_ray, objects, lights, depth - 1);
        if (!(le == BACKGROUND)) {
            Intersection ref_inter = test_ray(reflected_ray, objects, nullptr);
            Vector way = ref_inter.p - point;
            le *= inter.n.dot(way) / (inter.n.len() * way.len());
        }
        light_effect = le;
    } else {
        for (auto light : lights) {
            light_effect += light.calculate_effect(point, inter.n, inter.obj, objects);
        }
    }

    if (obj.props.reflective) {
        Vector reflected_vector = (ray.d - inter.n * 2 * ray.d.dot(inter.n)).normal();
        Ray reflected_ray(point + reflected_vector * eps, reflected_vector);
        reflected_color = trace_ray(reflected_ray, objects, lights, depth - 1);
        reflected_color *= obj.props.reflective;
    }

    if (obj.props.refractive) {
        Vector normal = inter.n;
        double cs = ray.d.dot(normal);
        double coef_from = 1;
        double coef_to = obj.props.refractive_coef;
        if (cs < 0) {
            cs *= -1;
        } else {
            normal *= -1;
            swap(coef_from, coef_to);
        }

        double ratio = coef_from / coef_to;
        double k = 1 - ratio * ratio * (1 - cs * cs);
        if (k >= 0) {
            Vector refracted_vector = ray.d * ratio + normal * (ratio * cs - sqrt(k));
            Ray refracted_ray(point + refracted_vector * eps, refracted_vector);
            refracted_color = trace_ray(refracted_ray, objects, lights, depth - 1);
            refracted_color *= obj.props.refractive;
        }
    }

    //reflected_color *= color;
    //refracted_color *= color;
    color *= light_effect * (1 - obj.props.reflective) * (1 - obj.props.refractive);
    color += reflected_color + refracted_color;
    return color;
}

vector<vector<Vector>> render_image(Camera camera, vector<Primitive*> objects, vector<Light> lights, int depth, int verbose) {
    vector<vector<Vector>> image(camera.res_x, vector<Vector>(camera.res_y));
    for (int x = 0; x < camera.res_x; ++x) {
        if (verbose) {
            if (x % (camera.res_x / 10) == 0) {
                cout << (x / (double)camera.res_x) * 100 << '%' << endl;
            }
        }
        for (int y = 0; y < camera.res_y; ++y) {
            if (false) {
                int cnt = 800;
                Vector color;
                for (int i = 0; i < cnt; ++i) {
                    Ray ray = camera.get_ray(x + uniform(gen), y + uniform(gen));
                    color += trace_ray(ray, objects, lights, depth);
                }
                image[x][y] = color / cnt;
            } else {
                Ray ray = camera.get_ray(x, y);
                image[x][y] = trace_ray(ray, objects, lights, depth);
            }
        }
    }
    return image;
}

void save_image(vector<vector<Vector>> &image, string filename) {
    int width = image.size();
    int height = image[0].size();
    ofstream ofs;
    ofs.open(filename, ofstream::out | ofstream::binary);
    ofs << "P6\n" << width << " " << height << "\n255\n";
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            ofs << (char)(255 * max(0.0, min(image[x][y].x, 1.0)));
            ofs << (char)(255 * max(0.0, min(image[x][y].y, 1.0)));
            ofs << (char)(255 * max(0.0, min(image[x][y].z, 1.0)));
        }
    }
    ofs.close();
}
