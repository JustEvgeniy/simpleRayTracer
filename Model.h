//
// Created by ju5t on 02.02.19.
//

#ifndef SIMPLERAYTRACER_MODEL_H
#define SIMPLERAYTRACER_MODEL_H

#include <ostream>
#include "geometry.h"
#include "Material.h"

class Model {
    std::vector<Vec3f> verts;
    std::vector<Vec3i> faces;
    Material material;
public:
    Model(const std::string &filename, const Material &m);

    const Material &getMaterial() const;

    int nverts() const;

    int nfaces() const;

    bool ray_triangle_intersect(const int &faceIdx, const Vec3f &origin, const Vec3f &dir,
                                float &tnear, Vec3f &N) const;

    const Vec3f &point(int i) const;

    Vec3f &point(int i);

    int vert(int fi, int li) const;

    void get_bbox(Vec3f &min, Vec3f &Max);

    friend std::ostream &operator<<(std::ostream &out, const Model &model);
};

#endif //SIMPLERAYTRACER_MODEL_H
