#include <iostream>
#include <exception>

#include "image_processor.h"

int main(int argc, char** argv) {
    try {
        ImageProcessor processor;

        processor.ParseArgs(argc, argv);
        processor.Process();
        processor.SaveOutputFile(argv[2]);

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}