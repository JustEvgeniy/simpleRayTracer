#include <iostream>
#include <vector>
#include <fstream>
#include <iostream>
#include <limits>
#include "geometry.h"

struct Sphere {
    Vec3f center;
    float radius;

    Sphere(const Vec3f &c, const float &r) : center(c), radius(r) {}

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

Vec3f cast_ray(const Vec3f &origin, const Vec3f &dir, const Sphere &sphere) {
    float sphereDist = std::numeric_limits<float>::max();
    if (!sphere.ray_intersect(origin, dir, sphereDist)) {
        return {0.2, 0.7, 0.8};
    }
    return {0.4, 0.4, 0.3};
}

void render(const Sphere &sphere) {
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
            frameBuffer[i + j * width] = cast_ray(center, dir, sphere);
        }
    }
    std::cerr << "Buffer filled\n";

    std::ofstream ofs;
    ofs.open("out.ppm");
    ofs << "P6\n" << width << ' ' << height << "\n255\n";
    for (int i = 0; i < height * width; ++i) {
        for (int j = 0; j < 3; ++j) {
            ofs << char(255 * std::max(0.f, std::min(1.f, frameBuffer[i][j])));
        }
    }
    ofs.close();
    std::cerr << "Image written\n";
}

int main() {
    Sphere sphere(Vec3f(2, 1, -5), 2);
    render(sphere);

    return 0;
}