#ifndef MODEL
#define MODEL

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include "vector.hpp"
#include "properties.hpp"
#include "primitive.hpp"

struct Model {
    Vector c;
    double coef;
    Vector rot;
    std::vector<Vector> points;
    std::vector<std::vector<int>> edges;
    Properties props;

    Model();
    Model(const Vector center, double coef_, const Properties &properties, std::string filename);

    std::vector<Triangle*> get_triangles();
};

std::istream& operator>>(std::istream& input_stream, Model &model);

#endif
