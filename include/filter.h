#pragma once

class Image;

class Filter {
public:
    virtual void Apply(Image& img) = 0;
    virtual ~Filter() = default;
};