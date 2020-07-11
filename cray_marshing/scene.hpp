#ifndef SCENE
#define SCENE

#pragma once

#include <vector>
#include <string>
#include <fstream>
#include <SFML/Graphics.hpp>
#include "vector.hpp"
#include "primitive.hpp"
#include "model.hpp"
#include "light.hpp"
#include "camera.hpp"

const int MAX_OBJ_COUNT = 100;

sf::Vector3f to_sf_vector(const Vector vector);

struct Scene {
    std::vector<Primitive*> objs;
    std::vector<Light> lights;
    Camera camera;
    int depth;
    int pixel_size;
    Sphere player;
    int to_display_player;
    Vector camera_offset;

    int spheres_cnt;
    sf::Vector3f spheres_center[MAX_OBJ_COUNT];
    float spheres_radius[MAX_OBJ_COUNT];

    int lights_cnt;
    sf::Vector3f lights_origin[MAX_OBJ_COUNT];
    sf::Vector3f lights_color[MAX_OBJ_COUNT];
    float lights_intensity[MAX_OBJ_COUNT];

    Scene();
    Scene(std::string filename);

    std::vector<std::vector<Vector>> render();
};

#endif // SCENE
