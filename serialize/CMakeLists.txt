cmake_minimum_required(VERSION 3.12)
project(serialize)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)

add_library(libserialize SHARED serialize.cpp)
target_link_libraries(libserialize PUBLIC cpputils)
target_include_directories(libserialize PUBLIC .)

add_executable(test_serialize test_serialize.cpp example.hpp)
target_link_libraries(test_serialize PUBLIC libserialize)
