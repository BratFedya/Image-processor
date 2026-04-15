cmake_minimum_required(VERSION 3.10)
project(image_processor)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

add_executable(image_processor
    image_processor.cpp
    src/image.cpp
    src/image_processor.cpp
    src/filters.cpp
)

target_compile_options(image_processor PRIVATE -Wall -Wextra -Werror)