#pragma once

#include <memory>
#include <string>
#include <vector>

#include "image.h"
#include "filter.h"

class ImageProcessor {
private:
    Image img_;
    std::vector<std::unique_ptr<Filter>> filters_;

public:
    void LoadInputFile(const std::string& path);
    void SaveOutputFile(const std::string& path) const;

    void AddFilter(std::unique_ptr<Filter> filter);
    void Process();

    void ParseArgs(int argc, char** argv);
};