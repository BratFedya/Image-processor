#include "filters.h"
#include "image.h"

#include <cmath>
#include <vector>
#include <stdexcept>

namespace {
const float K_GRAY_RED_COEFF = 0.299f;
const float K_GRAY_GREEN_COEFF = 0.587f;
const float K_GRAY_BLUE_COEFF = 0.114f;

const float K_COLOR_MAX_VALUE = 1.0f;
const float K_COLOR_MIN_VALUE = 0.0f;

const int K_SHARPEN_CENTER_COEFF = 5;
const int K_EDGE_CENTER_COEFF = 4;

const int K_GAUSSIAN_RADIUS_MULTIPLIER = 2;
const float K_GAUSSIAN_DENOMINATOR_COEFF = 2.0f;
}  // namespace

CropFilter::CropFilter(int width, int height) : width_(width), height_(height) {
    if (width <= 0 || height <= 0) {
        throw std::invalid_argument("Crop size must be positive");
    }
}

void CropFilter::Apply(Image& img) {
    int w = img.GetWidth();
    int h = img.GetHeight();

    int new_w = std::min(width_, w);
    int new_h = std::min(height_, h);

    Image result(new_w, new_h);

    for (int y = 0; y < new_h; ++y) {
        for (int x = 0; x < new_w; ++x) {
            result.At(x, y) = img.At(x, y);
        }
    }

    img = result;
}

void GrayscaleFilter::Apply(Image& img) {
    int w = img.GetWidth();
    int h = img.GetHeight();

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            Color& c = img.At(x, y);

            float gray = K_GRAY_RED_COEFF * c.r + K_GRAY_GREEN_COEFF * c.g + K_GRAY_BLUE_COEFF * c.b;

            c.r = c.g = c.b = gray;
        }
    }
}

void NegativeFilter::Apply(Image& img) {
    int w = img.GetWidth();
    int h = img.GetHeight();

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            Color& c = img.At(x, y);

            c.r = K_COLOR_MAX_VALUE - c.r;
            c.g = K_COLOR_MAX_VALUE - c.g;
            c.b = K_COLOR_MAX_VALUE - c.b;
        }
    }
}

void SharpenFilter::Apply(Image& img) {
    int w = img.GetWidth();
    int h = img.GetHeight();

    Image result(w, h);

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            Color center = img.At(x, y);
            Color& new_pixel = result.At(x, y);

            int up = (y + 1 < h) ? y + 1 : y;
            int down = (y - 1 >= 0) ? y - 1 : y;
            int left = (x - 1 >= 0) ? x - 1 : x;
            int right = (x + 1 < w) ? x + 1 : x;

            Color up_pixel = img.At(x, up);
            Color down_pixel = img.At(x, down);
            Color left_pixel = img.At(left, y);
            Color right_pixel = img.At(right, y);

            new_pixel.r = K_SHARPEN_CENTER_COEFF * center.r - up_pixel.r - down_pixel.r - left_pixel.r - right_pixel.r;
            new_pixel.g = K_SHARPEN_CENTER_COEFF * center.g - up_pixel.g - down_pixel.g - left_pixel.g - right_pixel.g;
            new_pixel.b = K_SHARPEN_CENTER_COEFF * center.b - up_pixel.b - down_pixel.b - left_pixel.b - right_pixel.b;

            new_pixel.Limit();
        }
    }

    img = result;
}

EdgeDetectionFilter::EdgeDetectionFilter(float t) : threshold_(t) {
    if (t < K_COLOR_MIN_VALUE || t > K_COLOR_MAX_VALUE) {
        throw std::invalid_argument("Edge requires a parameter between 0 and 1");
    }
}

void EdgeDetectionFilter::Apply(Image& img) {
    GrayscaleFilter gs;
    gs.Apply(img);

    int w = img.GetWidth();
    int h = img.GetHeight();

    Image result(w, h);

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            Color center = img.At(x, y);
            Color& new_pixel = result.At(x, y);

            int up = (y + 1 < h) ? y + 1 : y;
            int down = (y - 1 >= 0) ? y - 1 : y;
            int left = (x - 1 >= 0) ? x - 1 : x;
            int right = (x + 1 < w) ? x + 1 : x;

            Color up_pixel = img.At(x, up);
            Color down_pixel = img.At(x, down);
            Color left_pixel = img.At(left, y);
            Color right_pixel = img.At(right, y);

            float value = K_EDGE_CENTER_COEFF * center.r - up_pixel.r - down_pixel.r - left_pixel.r - right_pixel.r;

            if (value < K_COLOR_MIN_VALUE) {
                value = K_COLOR_MIN_VALUE;
            }
            if (value > K_COLOR_MAX_VALUE) {
                value = K_COLOR_MAX_VALUE;
            }

            if (value > threshold_) {
                new_pixel.r = new_pixel.g = new_pixel.b = K_COLOR_MAX_VALUE;
            } else {
                new_pixel.r = new_pixel.g = new_pixel.b = K_COLOR_MIN_VALUE;
            }
        }
    }

    img = result;
}

GaussianBlurFilter::GaussianBlurFilter(float s) : sigma_(s) {
    if (sigma_ <= 0) {
        throw std::invalid_argument("Sigma must be positive");
    }
}

void GaussianBlurFilter::Apply(Image& img) {
    int w = img.GetWidth();
    int h = img.GetHeight();

    Image result(w, h);

    int radius = std::ceil(K_GAUSSIAN_RADIUS_MULTIPLIER * sigma_);

    std::vector<std::vector<float>> kernel(2 * radius + 1, std::vector<float>(2 * radius + 1));
    float sum = 0.0f;

    for (int dy = -radius; dy <= radius; ++dy) {
        for (int dx = -radius; dx <= radius; ++dx) {
            float dist2 = static_cast<float>(dx * dx + dy * dy);
            float weight = std::exp(-dist2 / (K_GAUSSIAN_DENOMINATOR_COEFF * sigma_ * sigma_));
            kernel[dy + radius][dx + radius] = weight;
            sum += weight;
        }
    }

    for (int y = 0; y < 2 * radius + 1; ++y) {
        for (int x = 0; x < 2 * radius + 1; ++x) {
            kernel[y][x] /= sum;
        }
    }

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            float new_r = 0.0f;
            float new_g = 0.0f;
            float new_b = 0.0f;

            for (int dy = -radius; dy <= radius; ++dy) {
                for (int dx = -radius; dx <= radius; ++dx) {
                    int nx = x + dx;
                    int ny = y + dy;

                    if (nx < 0) {
                        nx = 0;
                    }
                    if (nx >= w) {
                        nx = w - 1;
                    }
                    if (ny < 0) {
                        ny = 0;
                    }
                    if (ny >= h) {
                        ny = h - 1;
                    }

                    const Color& neighbor = img.At(nx, ny);
                    float weight = kernel[dy + radius][dx + radius];

                    new_r += neighbor.r * weight;
                    new_g += neighbor.g * weight;
                    new_b += neighbor.b * weight;
                }
            }

            Color& result_pixel = result.At(x, y);
            result_pixel.r = new_r;
            result_pixel.g = new_g;
            result_pixel.b = new_b;
        }
    }

    img = result;
}