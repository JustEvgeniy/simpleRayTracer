//
// Created by ju5t on 02.02.19.
//

#include <fstream>
#include <sstream>
#include "Model.h"

// fills verts and faces arrays, supposes .obj file to have "f " entries without slashes
Model::Model(const std::string &filename, const Material &m) : verts(), faces(), material(m) {
    std::ifstream in;
    in.open(filename, std::ifstream::in);
    if (in.fail()) {
        std::cerr << "Failed to open " << filename << std::endl;
        return;
    }

    std::string line;
    while (!in.eof()) {
        std::getline(in, line);
        std::istringstream iss(line);
        char trash;
        if (!line.compare(0, 2, "v ")) {
            iss >> trash;
            Vec3f v;
            for (int i = 0; i < 3; i++)
                iss >> v[i];
            verts.push_back(v);
        } else if (!line.compare(0, 2, "f ")) {
            Vec3i f;
            int idx, cnt = 0;
            iss >> trash;
            while (iss >> idx) {
                idx--; // in wavefront obj all indices start at 1, not zero
                f[cnt++] = idx;
            }
            if (cnt == 3)
                faces.push_back(f);
        }
    }
    std::cout << "# v# " << verts.size() << " f# " << faces.size() << std::endl;

//    Vec3f min, max;
//    get_bbox(min, max);
}

// Moller and Trumbore
bool Model::ray_triangle_intersect(const int &faceIdx, const Vec3f &origin, const Vec3f &dir,
                                   float &tnear, Vec3f &N) const {
    Vec3f edge1 = point(vert(faceIdx, 1)) - point(vert(faceIdx, 0));
    Vec3f edge2 = point(vert(faceIdx, 2)) - point(vert(faceIdx, 0));
    Vec3f pvec = cross(dir, edge2);
    float det = edge1 * pvec;
    if (det < 1e-5)
        return false;

    Vec3f tvec = origin - point(vert(faceIdx, 0));
    float u = tvec * pvec;
    if (u < 0 || u > det)
        return false;

    Vec3f qvec = cross(tvec, edge1);
    float v = dir * qvec;
    if (v < 0 || u + v > det)
        return false;

    tnear = edge2 * qvec * (1.f / det);
    if (tnear > 1e-5) {
        N = cross(edge1, edge2);
        return true;
    }
    return false;
}


int Model::nverts() const {
    return static_cast<int>(verts.size());
}

int Model::nfaces() const {
    return static_cast<int>(faces.size());
}

void Model::get_bbox(Vec3f &min, Vec3f &max) {
    min = max = verts[0];
    for (int i = 1; i < nverts(); ++i) {
        for (int j = 0; j < 3; j++) {
            min[j] = std::min(min[j], verts[i][j]);
            max[j] = std::max(max[j], verts[i][j]);
        }
    }
    std::cout << "bbox: [" << min << " : " << max << "]" << std::endl;
}

const Vec3f &Model::point(int i) const {
//    assert(i >= 0 && i < nverts());
    return verts[i];
}

Vec3f &Model::point(int i) {
//    assert(i >= 0 && i < nverts());
    return verts[i];
}

int Model::vert(int fi, int li) const {
//    assert(fi >= 0 && fi < nfaces() && li >= 0 && li < 3);
    return faces[fi][li];
}

std::ostream &operator<<(std::ostream &out, const Model &m) {
    for (int i = 0; i < m.nverts(); i++) {
        out << "v " << m.point(i) << std::endl;
    }
    for (int i = 0; i < m.nfaces(); i++) {
        out << "f ";
        for (int k = 0; k < 3; k++) {
            out << (m.vert(i, k) + 1) << " ";
        }
        out << std::endl;
    }
    return out;
}

const Material &Model::getMaterial() const {
    return material;
}
