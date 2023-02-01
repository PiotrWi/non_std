cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

include_directories(
        ${PROJECT_SOURCE_DIR})

add_library(non_std
        containers/FibonacciHeap.h
        StringAlgorithms/algorithm.hpp
        StringAlgorithms/algorithm.cpp containers/HashMap.hpp)


