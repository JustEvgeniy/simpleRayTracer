//
// Created by ju5t on 02.02.19.
//

#ifndef SIMPLERAYTRACER_MATERIAL_H
#define SIMPLERAYTRACER_MATERIAL_H

struct Material {
    float refractiveIndex;
    Vec4f albedo;
    Vec3f diffuseColor;
    float specularExponent;

    Material() : refractiveIndex(1), albedo(1, 0, 0, 0), diffuseColor(), specularExponent() {}

    Material(const float r, const Vec4f &a, const Vec3f &c, const float s) :
            refractiveIndex(r), albedo(a), diffuseColor(c), specularExponent(s) {}
};

#endif //SIMPLERAYTRACER_MATERIAL_H
