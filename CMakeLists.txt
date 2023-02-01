cmake_minimum_required(VERSION 3.5)

project(nonStdTest LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(
        ${PROJECT_SOURCE_DIR}/non_std/)

add_subdirectory(non_std)

add_executable(nonStdTest main.cpp non_std/containers/HashTableWithRehashing.hpp)
target_link_libraries(nonStdTest
        non_std)
