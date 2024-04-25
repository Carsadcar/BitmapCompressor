cmake_minimum_required(VERSION 3.14)

project(ImageCompressor LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(ImageCompressor STATIC
  ImageCompressor.cpp
  ImageCompressor.h
)

target_compile_definitions(ImageCompressor PRIVATE IMAGECOMPRESSOR_LIBRARY)
