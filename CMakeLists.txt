cmake_minimum_required(VERSION 3.5)

project(nonStdTest LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR})
SET(CMAKE_CXX_FLAGS "-latomic")

add_subdirectory(non_std)

set (NON_STD_TESTING ON)

if(NON_STD_TESTING)
    add_executable(nonStdTest
            main.cpp
            #tests/lockFreeQueuesTests.cpp // Temporary skiped due to atomic linker errors....
            #tests/lockFreeQueuesTests.hpp
            tests/FixedSizeHashTableOpenHashingWIthAgeTests.hpp
            tests/FixedSizeHashTableOpenHashingWIthAgeTests.cpp)
    target_link_libraries(nonStdTest
            non_std)
endif()