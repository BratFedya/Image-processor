#pragma once

#include "filter.h"

class CropFilter : public Filter {
private:
    int width_;
    int height_;

public:
    CropFilter(int width, int height);
    void Apply(Image& img) override;
    ~CropFilter() override = default;
};

class GrayscaleFilter : public Filter {
public:
    void Apply(Image& img) override;
    ~GrayscaleFilter() override = default;
};

class NegativeFilter : public Filter {
public:
    void Apply(Image& img) override;
    ~NegativeFilter() override = default;
};

class SharpenFilter : public Filter {
public:
    void Apply(Image& img) override;
    ~SharpenFilter() override = default;
};

class EdgeDetectionFilter : public Filter {
private:
    float threshold_;

public:
    explicit EdgeDetectionFilter(float t);
    void Apply(Image& img) override;
    ~EdgeDetectionFilter() override = default;
};

class GaussianBlurFilter : public Filter {
private:
    float sigma_;

public:
    explicit GaussianBlurFilter(float s);
    void Apply(Image& img) override;
    ~GaussianBlurFilter() override = default;
};