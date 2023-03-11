cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

include_directories(
        ${PROJECT_SOURCE_DIR})

add_library(non_std
        StringAlgorithms/algorithm.cpp
        containers/FixedSizeHashTableOpenHashingWithAge.hpp containers/Traits.hpp internal/Logger.hpp)


