#include "image.h"

#include <fstream>
#include <vector>
#include <string>
#include <cmath>
#include <stdexcept>
#include <cstdint>

namespace {
constexpr int KZero = 0;
constexpr int KOne = 1;
constexpr int KTwo = 2;
constexpr int KThree = 3;
constexpr int KFour = 4;
constexpr int KEight = 8;
constexpr int KFourteen = 14;
constexpr int KTwenty = 20;
constexpr int KTwentyFour = 24;
constexpr int KForty = 40;
constexpr int KBmpSignature = 0x4D42;
constexpr int KPixelsPerMeter = 2835;
constexpr float KColorScale = 255.f;
}  // namespace

Image::Image() : width_(0), height_(0) {
}

Image::Image(int w, int h) : width_(w), height_(h), pixels_(w * h) {
}

int Image::GetWidth() const {
    return width_;
}

int Image::GetHeight() const {
    return height_;
}

Color& Image::At(int x, int y) {
    if (x < KZero) {
        x = KZero;
    }
    if (x >= width_) {
        x = width_ - KOne;
    }
    if (y < KZero) {
        y = KZero;
    }
    if (y >= height_) {
        y = height_ - KOne;
    }

    return pixels_[y * width_ + x];
}

const Color& Image::At(int x, int y) const {
    if (x < KZero) {
        x = KZero;
    }
    if (x >= width_) {
        x = width_ - KOne;
    }
    if (y < KZero) {
        y = KZero;
    }
    if (y >= height_) {
        y = height_ - KOne;
    }

    return pixels_[y * width_ + x];
}

void Image::LoadBMP(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Cannot open file: " + filename);
    }

    // File header
    uint16_t bf_type = 0;
    file.read(reinterpret_cast<char*>(&bf_type), KTwo);
    if (bf_type != KBmpSignature) {
        throw std::runtime_error("Not a BMP file");
    }

    file.ignore(KEight);
    uint32_t bf_off_bits = 0;
    file.read(reinterpret_cast<char*>(&bf_off_bits), KFour);

    // DIB header
    uint32_t bi_size = 0;
    file.read(reinterpret_cast<char*>(&bi_size), KFour);
    if (bi_size != KForty) {
        throw std::runtime_error("Unsupported header size");
    }

    int32_t w = 0;
    int32_t h = 0;

    file.read(reinterpret_cast<char*>(&w), KFour);
    file.read(reinterpret_cast<char*>(&h), KFour);
    width_ = w;
    height_ = std::abs(h);
    pixels_.resize(width_ * height_);

    uint16_t planes = 0;
    uint16_t bit_count = 0;

    file.read(reinterpret_cast<char*>(&planes), KTwo);
    file.read(reinterpret_cast<char*>(&bit_count), KTwo);
    if (bit_count != KTwentyFour) {
        throw std::runtime_error("Only 24-bit BMP supported");
    }

    uint32_t compression = 0;
    file.read(reinterpret_cast<char*>(&compression), KFour);
    if (compression != KZero) {
        throw std::runtime_error("Compressed BMP not supported");
    }

    file.ignore(KTwenty);

    file.seekg(bf_off_bits, std::ios::beg);

    int row_padded = (width_ * KThree + KThree) & (~KThree);
    std::vector<uint8_t> row(row_padded);

    bool flip_vertically = h > KZero;

    for (int y = 0; y < height_; ++y) {
        file.read(reinterpret_cast<char*>(row.data()), row_padded);
        int row_index = flip_vertically ? height_ - KOne - y : y;
        for (int x = 0; x < width_; ++x) {
            uint8_t b = row[x * KThree];
            uint8_t g = row[x * KThree + KOne];
            uint8_t r = row[x * KThree + KTwo];
            const float scale = static_cast<float>(KColorScale);

            pixels_[row_index * width_ + x] =
                Color(static_cast<float>(r) / scale, static_cast<float>(g) / scale, static_cast<float>(b) / scale);
        }
    }
}

void Image::SaveBMP(const std::string& filename) const {
    std::ofstream file(filename, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Cannot create file: " + filename);
    }

    int row_padded = (width_ * KThree + KThree) & (~KThree);
    uint32_t filesize = KFourteen + KForty + row_padded * height_;

    uint16_t bf_type = KBmpSignature;
    file.write(reinterpret_cast<const char*>(&bf_type), KTwo);

    uint32_t bf_size = filesize;
    file.write(reinterpret_cast<const char*>(&bf_size), KFour);

    uint16_t bf_reserved1 = 0;
    uint16_t bf_reserved2 = 0;
    file.write(reinterpret_cast<const char*>(&bf_reserved1), KTwo);
    file.write(reinterpret_cast<const char*>(&bf_reserved2), KTwo);

    uint32_t bf_off_bits = KFourteen + KForty;
    file.write(reinterpret_cast<const char*>(&bf_off_bits), KFour);

    uint32_t bi_size = KForty;
    int32_t w = width_;
    int32_t h = height_;
    uint16_t planes = KOne;
    uint16_t bit_count = KTwentyFour;
    uint32_t compression = 0;
    uint32_t image_size = row_padded * height_;
    int32_t x_pixels_per_meter = KPixelsPerMeter;
    int32_t y_pixels_per_meter = KPixelsPerMeter;
    uint32_t clr_used = 0;
    uint32_t clt_important = 0;

    file.write(reinterpret_cast<const char*>(&bi_size), KFour);
    file.write(reinterpret_cast<const char*>(&w), KFour);
    file.write(reinterpret_cast<const char*>(&h), KFour);
    file.write(reinterpret_cast<const char*>(&planes), KTwo);
    file.write(reinterpret_cast<const char*>(&bit_count), KTwo);
    file.write(reinterpret_cast<const char*>(&compression), KFour);
    file.write(reinterpret_cast<const char*>(&image_size), KFour);
    file.write(reinterpret_cast<const char*>(&x_pixels_per_meter), KFour);
    file.write(reinterpret_cast<const char*>(&y_pixels_per_meter), KFour);
    file.write(reinterpret_cast<const char*>(&clr_used), KFour);
    file.write(reinterpret_cast<const char*>(&clt_important), KFour);

    std::vector<uint8_t> row(row_padded, 0);

    for (int y = 0; y < height_; ++y) {
        std::fill(row.begin(), row.end(), 0);

        int row_index = height_ - KOne - y;
        for (int x = 0; x < width_; ++x) {
            Color c = pixels_[row_index * width_ + x];
            c.Limit();

            row[x * KThree] = static_cast<uint8_t>(std::round(c.b * KColorScale));
            row[x * KThree + KOne] = static_cast<uint8_t>(std::round(c.g * KColorScale));
            row[x * KThree + KTwo] = static_cast<uint8_t>(std::round(c.r * KColorScale));
        }

        file.write(reinterpret_cast<const char*>(row.data()), row_padded);
    }

    if (!file) {
        throw std::runtime_error("Failed while writing BMP file: " + filename);
    }
};