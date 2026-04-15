#pragma once

#include <vector>
#include <string>

struct Color {
    float r;
    float g;
    float b;

    Color() : r(0), g(0), b(0) {
    }
    Color(float red, float green, float blue) : r(red), g(green), b(blue) {
    }

    Color operator+(const Color& other) const {
        return Color(r + other.r, g + other.g, b + other.b);
    }
    Color operator*(float k) const {
        return Color(r * k, g * k, b * k);
    }

    void Limit() {
        if (r > 1) {
            r = 1;
        }
        if (g > 1) {
            g = 1;
        }
        if (b > 1) {
            b = 1;
        }
        if (r < 0) {
            r = 0;
        }
        if (g < 0) {
            g = 0;
        }
        if (b < 0) {
            b = 0;
        }
    }
};

class Image {
private:
    int width_;
    int height_;
    std::vector<Color> pixels_;

public:
    Image();
    Image(int w, int h);

    int GetWidth() const;
    int GetHeight() const;

    Color& At(int x, int y);
    const Color& At(int x, int y) const;

    void LoadBMP(const std::string& filename);
    void SaveBMP(const std::string& filename) const;
};