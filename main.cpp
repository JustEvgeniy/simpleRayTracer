#include <iostream>
#include <vector>
#include <fstream>
#include <iostream>
#include <limits>
#include "geometry.h"

struct Material {
    Vec3f albedo;
    Vec3f diffuseColor;
    float specularExponent;

    Material() : albedo(1, 0, 0), diffuseColor(), specularExponent() {}

    Material(const Vec3f &a, const Vec3f &c, const float &s) : albedo(a), diffuseColor(c), specularExponent(s) {}
};

struct Light {
    Vec3f position;
    float intensity;

    Light(const Vec3f &p, const float &i) : position(p), intensity(i) {}
};

struct Sphere {
    Vec3f center;
    float radius;
    Material material;

    Sphere(const Vec3f &c, const float &r, const Material &m) : center(c), radius(r), material(m) {}

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
    return spheresDist < 1000;
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
    Vec3f reflectOrigin = reflectDir * N < 0 ? point - N * 1e-4 : point + N * 1e-4;
    Vec3f reflectColor = cast_ray(reflectOrigin, reflectDir, spheres, lights, depth + 1);

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
           reflectColor * material.albedo[2];
}

void render(const std::vector<Sphere> &spheres, const std::vector<Light> &lights) {
    const int width = 1024;
    const int height = 768;
    std::vector<Vec3f> frameBuffer(width * height);
    std::cerr << "Buffer created\n";

    const float fov = 90 * M_PI / 180;
    Vec3f center(0, 0, 0);

    for (int i = 0; i < width; ++i) {
        for (int j = 0; j < height; ++j) {
            float x = (2 * (i + 0.5f) / float(width) - 1) * tanf(fov / 2) * width / float(height);
            float y = -(2 * (j + 0.5f) / float(height) - 1) * tanf(fov / 2);
            Vec3f dir = Vec3f(x, y, -1).normalize();
            frameBuffer[i + j * width] = cast_ray(center, dir, spheres, lights);
        }
    }
    std::cerr << "Buffer filled\n";

    std::ofstream ofs;
    ofs.open("out.ppm");
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
    std::cerr << "Image written\n";
}

int main() {
    Material ivory({0.6, 0.3, 0.1}, {0.4, 0.4, 0.3}, 50);
    Material redRubber({0.9, 0.1, 0}, {0.3, 0.1, 0.1}, 10);
    Material mirror({0, 10, 0.8}, {1, 1, 1}, 1425);

    std::vector<Sphere> spheres;
//    spheres.emplace_back(Vec3f(3, 3, -4), 1, redRubber);
//    spheres.emplace_back(Vec3f(3, 3, -10), 3.3333, mirror);
//    spheres.emplace_back(Vec3f(3, -3, -4), 0.1, ivory);
//    spheres.emplace_back(Vec3f(3, -3, -10), 1, redRubber);
//    spheres.emplace_back(Vec3f(-3, 3, -4), 1, ivory);
//    spheres.emplace_back(Vec3f(-3, 3, -10), 0.5, mirror);
//    spheres.emplace_back(Vec3f(-3, -3, -4), 1, redRubber);
//    spheres.emplace_back(Vec3f(-3, -3, -10), 1, ivory);


    spheres.emplace_back(Vec3f(3, 3, -4), 3, mirror);
    spheres.emplace_back(Vec3f(3, 3, -10), 3, mirror);
    spheres.emplace_back(Vec3f(3, -3, -4), 3, mirror);
    spheres.emplace_back(Vec3f(3, -3, -10), 3, mirror);
    spheres.emplace_back(Vec3f(-3, 3, -4), 3, mirror);
    spheres.emplace_back(Vec3f(-3, 3, -10), 3, mirror);
    spheres.emplace_back(Vec3f(-3, -3, -4), 3, mirror);
    spheres.emplace_back(Vec3f(-3, -3, -10), 3, mirror);

    std::vector<Light> lights;
    lights.emplace_back(Vec3f(0, 0, -7), 1.6);
    lights.emplace_back(Vec3f(20, 20, 30), 3);

    render(spheres, lights);

    return 0;
}