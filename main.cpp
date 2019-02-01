#include <iostream>
#include <vector>
#include <fstream>
#include <iostream>
#include "geometry.h"

void render() {
    const int width = 1024;
    const int height = 768;
    std::vector<Vec3f> frameBuffer(width * height);
    std::cerr << "Buffer created\n";

    for (int j = 0; j < height; ++j) {
        for (int i = 0; i < width; ++i) {
            frameBuffer[i + j * width] = Vec3f(j / float(height), i / float(width), 0);
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
    render();
    return 0;
}