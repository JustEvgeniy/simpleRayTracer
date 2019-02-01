#include <iostream>
#include <vector>
#include <fstream>
#include <iostream>
#include <limits>
#include "geometry.h"

struct Material {
    float refractiveIndex;
    Vec4f albedo;
    Vec3f diffuseColor;
    float specularExponent;

    Material() : refractiveIndex(1), albedo(1, 0, 0, 0), diffuseColor(), specularExponent() {}

    Material(const float r, const Vec4f &a, const Vec3f &c, const float s) :
            refractiveIndex(r), albedo(a), diffuseColor(c), specularExponent(s) {}
};

struct Light {
    Vec3f position;
    float intensity;

    Light(const Vec3f &p, const float i) : position(p), intensity(i) {}
};

struct Sphere {
    Vec3f center;
    float radius;
    Material material;

    Sphere(const Vec3f &c, const float r, const Material &m) : center(c), radius(r), material(m) {}

    bool ray_intersect(const Vec3f &origin, const Vec3f &dir, float &t0) const {
        Vec3f L = center - origin;
        float tca = L * dir;
        float d2 = L * L - tca * tca;
        if (d2 > radius * radius)
            return false;

        float thc = sqrtf(radius * radius - d2);
        t0 = tca - thc;
        float t1 = tca + thc;
        if (t0 < 0)
            t0 = t1;
        return t0 >= 0;
    }
};

Vec3f reflect(const Vec3f &I, const Vec3f &N) {
    return N * 2 * (I * N) - I;
}

Vec3f refract(const Vec3f &I, const Vec3f &N, const float eta_t, const float eta_i = 1.f) {
    float cosi = -std::max(-1.f, std::min(1.f, I * N));
    // if the ray comes from the inside the object, swap the air and the media
    if (cosi < 0)
        return refract(I, -N, eta_i, eta_t);

    float eta = eta_i / eta_t;
    float k = 1 - eta * eta * (1 - cosi * cosi);
    // k < 0 = total reflection, no ray to refract. I refract it anyways, this has no physical meaning
    return k < 0 ? Vec3f(1, 0, 0) : I * eta + N * (eta * cosi - sqrtf(k));
}

bool scene_intersect(const Vec3f &origin, const Vec3f &dir, const std::vector<Sphere> &spheres,
                     Vec3f &hit, Vec3f &N, Material &material) {
    float spheresDist = std::numeric_limits<float>::max();
    for (const auto &sphere : spheres) {
        float dist_i;
        if (sphere.ray_intersect(origin, dir, dist_i) && dist_i < spheresDist) {
            spheresDist = dist_i;
            hit = origin + dir * dist_i;
            N = (hit - sphere.center).normalize();
            material = sphere.material;
        }
    }

    float checkerboardDist = std::numeric_limits<float>::max();
    if (fabs(dir.y) > 1e-4) {
        float d = -(origin.y + 4) / dir.y;
        Vec3f pt = origin + dir * d;
        if (d > 0 && d < spheresDist &&
            fabs(pt.x) < 100 &&
            fabs(pt.z) < 100) {
            checkerboardDist = d;
            hit = pt;
            N = Vec3f(0, 1, 0);
            material.diffuseColor = (int(.5 * hit.x + 1000) + int(.5 * hit.z)) % 2 ?
                                    Vec3f(.3, .3, .3) :
                                    Vec3f(.3, .2, .1);
        }
    }

    return std::min(spheresDist, checkerboardDist) < 1000;
}

Vec3f cast_ray(const Vec3f &origin, const Vec3f &dir,
               const std::vector<Sphere> &spheres,
               const std::vector<Light> &lights,
               size_t depth = 0) {
    Vec3f point, N;
    Material material;
    if (depth > 4 || !scene_intersect(origin, dir, spheres, point, N, material)) {
        return {0.2, 0.7, 0.8};
    }

    Vec3f reflectDir = reflect(-dir, N).normalize();
    Vec3f refractDir = refract(dir, N, material.refractiveIndex).normalize();
    Vec3f reflectOrigin = reflectDir * N < 0 ? point - N * 1e-4 : point + N * 1e-4;
    Vec3f refractOrigin = refractDir * N < 0 ? point - N * 1e-4 : point + N * 1e-4;
    Vec3f reflectColor = cast_ray(reflectOrigin, reflectDir, spheres, lights, depth + 1);
    Vec3f refractColor = cast_ray(refractOrigin, refractDir, spheres, lights, depth + 1);

    float diffuseLightIntensity = 0;
    float specularLightIntensity = 0;
    for (const auto &light : lights) {
        Vec3f lightDir = (light.position - point).normalize();
        float lightDistance = (light.position - point).norm();

        Vec3f shadowOrigin = lightDir * N < 0 ? point - N * 1e-4 : point + N * 1e-4;
        Vec3f shadowPt, shadowN;
        Material tmpMat;
        if (scene_intersect(shadowOrigin, lightDir, spheres, shadowPt, shadowN, tmpMat) &&
            (shadowPt - shadowOrigin).norm() < lightDistance)
            continue;

        diffuseLightIntensity += light.intensity * std::max(0.f, lightDir * N);
        specularLightIntensity +=
                light.intensity * powf(std::max(0.f, reflect(lightDir, N) * -dir), material.specularExponent);
    }

    return material.diffuseColor * diffuseLightIntensity * material.albedo[0] +
           Vec3f(1, 1, 1) * specularLightIntensity * material.albedo[1] +
           reflectColor * material.albedo[2] +
           refractColor * material.albedo[3];
}

void render(const std::vector<Sphere> &spheres, const std::vector<Light> &lights) {
    const int width = 1024;
    const int height = 768;
//    const int width = 1920 * 4;
//    const int height = 1080 * 4;
    std::vector<Vec3f> frameBuffer(width * height);
    std::cout << "Buffer created\n";

    const float fovDeg = 60;
    const float fov = fovDeg * M_PI / 180;
    Vec3f center(0, 0, 0);

#pragma omp parallel for
    for (int i = 0; i < width; ++i) {
        for (int j = 0; j < height; ++j) {
            float dirX = (i + 0.5f) - width / 2.f;
            float dirY = -(j + 0.5f) + height / 2.f;
            float dirZ = -height / (2 * tanf(fov / 2));
            Vec3f dir = Vec3f(dirX, dirY, dirZ).normalize();
            frameBuffer[i + j * width] = cast_ray(center, dir, spheres, lights);
        }
    }
    std::cout << "Buffer filled\n";

    std::ofstream ofs;
    ofs.open("out.ppm", std::ios::binary);
    ofs << "P6\n" << width << ' ' << height << "\n255\n";
    for (int i = 0; i < height * width; ++i) {
        Vec3f &c = frameBuffer[i];
        float max = std::max(c[0], std::max(c[1], c[2]));
        if (max > 1)
            c = c * (1. / max);
        for (int j = 0; j < 3; ++j) {
            ofs << char(255 * std::max(0.f, std::min(1.f, frameBuffer[i][j])));
//            ofs << char(255 * frameBuffer[i][j]);
        }
    }
    ofs.close();
    std::cout << "Image written\n";
}

int main() {
    Material ivory(1, Vec4f(0.6, 0.3, 0.1, 0.0), Vec3f(0.4, 0.4, 0.3), 50);
    Material glass(1.5, Vec4f(0.0, 0.5, 0.1, 0.8), Vec3f(0.6, 0.7, 0.8), 125);
    Material redRubber(1, Vec4f(0.9, 0.1, 0.0, 0.0), Vec3f(0.3, 0.1, 0.1), 10);
    Material mirror(1, Vec4f(0.0, 10.0, 0.8, 0.0), Vec3f(1.0, 1.0, 1.0), 1425);


    std::vector<Sphere> spheres;
    spheres.emplace_back(Vec3f(-3, 0, -16), 2, ivory);
    spheres.emplace_back(Vec3f(-1.0f, -1.5f, -12), 2, glass);
    spheres.emplace_back(Vec3f(1.5, -0.5f, -18), 3, redRubber);
    spheres.emplace_back(Vec3f(7, 5, -18), 4, mirror);

    std::vector<Light> lights;
    lights.emplace_back(Vec3f(-20, 20, 20), 1.3);
    lights.emplace_back(Vec3f(30, 50, -25), 1.5);
    lights.emplace_back(Vec3f(30, 20, 30), 1.9);

    render(spheres, lights);

    return 0;
}