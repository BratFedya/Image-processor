#include "image_processor.h"

#include <stdexcept>
#include <string>
#include <memory>

#include "filters.h"

void ImageProcessor::LoadInputFile(const std::string& path) {
    try {
        img_.LoadBMP(path);
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to load BMP file '" + path + "': " + e.what());
    }
}

void ImageProcessor::SaveOutputFile(const std::string& path) const {
    try {
        img_.SaveBMP(path);
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to save BMP file '" + path + "': " + e.what());
    }
}

void ImageProcessor::AddFilter(std::unique_ptr<Filter> f) {
    filters_.push_back(std::move(f));
}

void ImageProcessor::Process() {
    for (const auto& filter : filters_) {
        filter->Apply(img_);
    }
}

void ImageProcessor::ParseArgs(int argc, char** argv) {
    if (argc < 3) {
        throw std::invalid_argument("Usage: image_processor <input.bmp> <output.bmp> [filters]");
    }

    LoadInputFile(argv[1]);

    int i = 3;
    while (i < argc) {
        std::string filter_name = argv[i];

        if (filter_name == "-crop") {
            if (i + 2 >= argc) {
                throw std::invalid_argument("Filter -crop requires 2 parameters: width height");
            }

            int width = std::stoi(argv[i + 1]);
            int height = std::stoi(argv[i + 2]);

            AddFilter(std::make_unique<CropFilter>(width, height));
            i += 3;
        } else if (filter_name == "-gs") {
            AddFilter(std::make_unique<GrayscaleFilter>());
            i += 1;
        } else if (filter_name == "-neg") {
            AddFilter(std::make_unique<NegativeFilter>());
            i += 1;
        } else if (filter_name == "-sharp") {
            AddFilter(std::make_unique<SharpenFilter>());
            i += 1;
        } else if (filter_name == "-edge") {
            if (i + 1 >= argc) {
                throw std::invalid_argument("Filter -edge requires 1 parameter: threshold");
            }

            float threshold = std::stof(argv[i + 1]);

            AddFilter(std::make_unique<EdgeDetectionFilter>(threshold));
            i += 2;
        } else if (filter_name == "-blur") {
            if (i + 1 >= argc) {
                throw std::invalid_argument("Filter -blur requires 1 parameter: sigma");
            }

            float sigma = std::stof(argv[i + 1]);

            AddFilter(std::make_unique<GaussianBlurFilter>(sigma));
            i += 2;
        } else {
            throw std::invalid_argument("Unknown filter: " + filter_name);
        }
    }
}
// a