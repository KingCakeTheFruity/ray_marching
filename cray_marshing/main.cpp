#pragma GCC optimize ("O3")

#include <algorithm>
#include <chrono>
#include <string>
#include "ray_tracer.hpp"
#include <SFML/Graphics.hpp>

std::vector<std::vector<sf::RectangleShape>> image_to_rects(std::vector<std::vector<Vector>>& image, int rect_size) {
    std::vector<std::vector<sf::RectangleShape>> ret(image.size(), std::vector<sf::RectangleShape>(image[0].size(), sf::RectangleShape(sf::Vector2f(rect_size, rect_size))));
    for (int i = 0; i < image.size(); ++i) {
        for (int j = 0; j < image[0].size(); ++j) {
            ret[i][j].setPosition(i * rect_size, j * rect_size);
            int r = (255 * std::max(0.0, std::min(image[i][j].x, 1.0)));
            int g = (255 * std::max(0.0, std::min(image[i][j].y, 1.0)));
            int b = (255 * std::max(0.0, std::min(image[i][j].z, 1.0)));
            ret[i][j].setFillColor(sf::Color(r, g, b));
        }
    }
    return ret;
}

void take_screenshot(const sf::RenderWindow& window, const std::string& filename)
{
    sf::Texture texture;
    texture.create(window.getSize().x, window.getSize().y);
    texture.update(window);
    texture.copyToImage().saveToFile(filename);
}

int main() {
    const bool MOUSE_LOCK = false;
    std::string filename = "scene.txt";
    Scene scene(filename);

    int rect_size = scene.pixel_size;

    int w = scene.camera.res_x * scene.pixel_size;
    int h = scene.camera.res_y * scene.pixel_size;
    cout << w << ' ' << h << endl;

    sf::RenderWindow window(sf::VideoMode(w, h), "KCTF | FPS: ");

    int mx = w / 2;
    int my = h / 2;
    double sense = 0.1;
    sf::Mouse::setPosition(sf::Vector2i(mx, my), window);
    Vector offset(0, 0, 0);

    double pi = 3.1415926535;
    double speed = 0.1;
    Vector gravity = {0, 0, -speed * 0.35};

    auto init_time = std::chrono::system_clock::now();
    auto sec_start = std::chrono::system_clock::now();
    auto cur_time = std::chrono::system_clock::now();
    int  frames_cnt = 0;

    // SPRITES, TEXTURES, SHADER ===

    sf::Sprite sprite;
    sprite.setPosition(0, 0);

    sf::Texture texture;
    texture.create(scene.camera.res_x, scene.camera.res_y);
    sprite.setTexture(texture);

    sf::RenderTexture scaler;
    scaler.create(scene.camera.res_x, scene.camera.res_y);
    sf::Sprite scaled(scaler.getTexture());
    scaled.setPosition(0, 0);
    scaled.setScale(scene.pixel_size, scene.pixel_size);

    sf::Shader shader;
    shader.loadFromFile("shader.frag", sf::Shader::Fragment);

    //===============================

    while (window.isOpen()) {
        cur_time = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_seconds = cur_time - sec_start;
        double dt = elapsed_seconds.count();
        if (dt >= 1.0) {
            window.setTitle("KCTF | FPS: " + to_string(frames_cnt));
            sec_start = cur_time;
            frames_cnt = 0;
        }
        ++frames_cnt;

        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        int xxx, yyy, zzz, ddd;
        xxx = scene.player.c.x;
        yyy = scene.player.c.y;
        zzz = scene.player.c.z;
        ddd = scene.camera.dist;

        Vector shift;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
            shift += scene.camera.d;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
            shift += scene.camera.d * -1;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
            shift += scene.camera.ort1;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
            shift += scene.camera.ort1 * -1;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Z)) {
            scene.camera.dist -= 1;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::X)) {
            scene.camera.dist += 1;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::C)) {
            scene.camera.dist = 100;
        }
        shift.z = 0;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::E)) {
            shift += Vector(0, 0, 1.5);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q)) {
            shift += Vector(0, 0, -1.5);
        }

        shift = shift.normal() * speed;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) {
            shift *= 2;
        }
        Vector init = scene.player.c;
        //shift += gravity;
        scene.player.c += shift;
        Vector shifted = shift;

        std::sort(scene.objs.begin(), scene.objs.end(),
            [scene](Primitive* a, Primitive* b) {
                return scene.player.dist(a) < scene.player.dist(b);
            });

        for (auto obj : scene.objs) {
            Vector inter_norm = scene.player.intersects(obj);
            if (inter_norm.len()) {
                double t = shift.dot(inter_norm) / inter_norm.len();
                scene.player.c += inter_norm;
            }
        }
        scene.camera.o = scene.player.c + scene.camera_offset;

        // CAMERA ROTATION

        sf::Vector2i mxy = sf::Mouse::getPosition(window);
        int cur_mx = mxy.x;
        int cur_my = mxy.y;
        int cur_dx = cur_mx - mx;
        int cur_dy = cur_my - my;

        double dx = cur_dx * sense;
        double dy = cur_dy * sense;

        double cx = scene.camera.d.x;
        double cy = scene.camera.d.y;
        double cz = scene.camera.d.z;
        double cyy = cy / (cy * cy + cx * cx);
        double cxx = cx / (cy * cy + cx * cx);

        if (MOUSE_LOCK || true) {
            sf::Mouse::setPosition(sf::Vector2i(mx, my), window);
            scene.camera.d = rotz(scene.camera.d, +pi * dx / 130);
            scene.camera.d = roty(scene.camera.d, -pi * dy * cxx / 130);
            scene.camera.d = rotx(scene.camera.d, +pi * dy * cyy / 130);
        }

        scene.camera.update();

        // DRAWING

        std::chrono::duration<float> time_from_start = cur_time - init_time;
        float time = time_from_start.count();
        shader.setUniform("time", time);

        shader.setUniform("origin", to_sf_vector(scene.camera.o));
        shader.setUniform("ort1", to_sf_vector(scene.camera.ort1));
        shader.setUniform("ort2", to_sf_vector(scene.camera.ort2));
        shader.setUniform("left_upper", to_sf_vector(scene.camera.left_upper));
        shader.setUniform("w", (float)scene.camera.w);
        shader.setUniform("h", (float)scene.camera.h);
        shader.setUniform("res_x", (float)scene.camera.res_x);
        shader.setUniform("res_y", (float)scene.camera.res_y);

        shader.setUniform("spheres_count", scene.spheres_cnt);
        shader.setUniformArray("sph_o", scene.spheres_center, MAX_OBJ_COUNT);
        shader.setUniformArray("sph_r", scene.spheres_radius, MAX_OBJ_COUNT);

        shader.setUniform("lights_count", scene.lights_cnt);
        shader.setUniformArray("light_o", scene.lights_origin, MAX_OBJ_COUNT);
        shader.setUniformArray("light_color", scene.lights_color, MAX_OBJ_COUNT);
        shader.setUniformArray("light_intensity", scene.lights_intensity, MAX_OBJ_COUNT);

        scaler.draw(sprite, &shader);
        scaler.display();
        window.draw(scaled);
        window.display();

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::F)) {
            take_screenshot(window, "screenshot.png");
        }
    }

    return 0;
}
