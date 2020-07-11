#include "model.hpp"

Model::Model() {}

Model::Model(const Vector center, double coef_, const Properties &properties, std::string filename) {
    c = center;
    coef = coef_;
    props = properties;

    std::fstream fs;
    fs.open(filename, std::fstream::in);
    std::string type;
    while (fs >> type) {
        if (type == "p") {
            double x, y, z;
            fs >> x >> y >> z;
            points.push_back({x, y, z});
        } else if (type == "e") {
            int i1, i2, i3;
            fs >> i1 >> i2 >> i3;
            --i1;
            --i2;
            --i3;
            edges.push_back({i1, i2, i3});
        }
    }
}

std::vector<Triangle*> Model::get_triangles() {
    Vector rot = props.rotation;
    std::vector<Triangle*> triags;
    for (auto edge : edges) {
        int f, s, t;
        f = edge[0];
        s = edge[1];
        t = edge[2];
        Vector p1 = c + rotate(points[f], rot) * coef;
        Vector p2 = c + rotate(points[s], rot) * coef;
        Vector p3 = c + rotate(points[t], rot) * coef;
        Triangle *triag = new Triangle(p1, p2, p3, props);
        triags.push_back(triag);
    }
    return triags;
}

std::istream& operator>>(std::istream& input_stream, Model &model) {
    std::string filename;
    Vector c;
    double coef;
    Properties props;
    input_stream >> filename >> c >> coef >> props;
    model = Model(c, coef, props, filename);
    return input_stream;
}
