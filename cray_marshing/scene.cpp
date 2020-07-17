#include "scene.hpp"

sf::Vector3f to_sf_vector(const Vector vector) {
    return { (float)vector.x, (float)vector.y, (float)vector.z };
}

Scene::Scene() {}

Scene::Scene(std::string filename) {
    std::fstream fs;
    fs.open(filename, std::fstream::in);
    std::string s;
    int width;
    int height;
    fs >> s >> width >> s >> height;
    double screen_distance;
    fs >> s >> screen_distance;
    double resolution_coef;
    fs >> s >> resolution_coef;
    fs >> s >> pixel_size;

    fs >> s >> depth;

    Vector cam_pos;
    fs >> s >> cam_pos;
    Vector cam_dir;
    fs >> s >> cam_dir;
    camera = Camera(cam_pos, cam_dir, screen_distance, width, height, resolution_coef);

    fs >> s >> player;
    fs >> s >> to_display_player;
    fs >> s >> camera_offset;

    spheres_cnt = -1;
    lights_cnt = -1;

    std::string type;
    while (fs >> type) {
        if (type == "Primitive") {
            Primitive* prim;
            fs >> prim;
            objs.push_back(prim);

            if (prim->type == SPHERE) {
                Sphere* sphere = dynamic_cast<Sphere*>(prim);
                spheres_cnt++;
                spheres_center[spheres_cnt] = to_sf_vector(sphere->c);
                spheres_radius[spheres_cnt] = sphere->r;
            }
        } else if (type == "Light") {
            Light light;
            fs >> light;
            lights.push_back(light);
            
            lights_cnt++;
            lights_origin[lights_cnt] = to_sf_vector(light.o);
            lights_color[lights_cnt] = to_sf_vector(light.color);
            lights_intensity[lights_cnt] = light.distance_coef;
            lights_stable_distances[lights_cnt] = light.stable_distance;
        } else if (type == "Model") {
            Model model;
            fs >> model;
            for (Triangle *triag : model.get_triangles()) {
                objs.push_back(triag);
            }
        }
    }
}

std::vector<std::vector<Vector>> render_image(Camera camera, std::vector<Primitive*> objects, std::vector<Light> lights, int depth, int verbose);

std::vector<std::vector<Vector>> Scene::render() {
    return render_image(camera, objs, lights, depth, 0);
}
