cmake_minimum_required(VERSION 3.13)
project(half)

set(CMAKE_CXX_STANDARD 14)

add_executable(
        half
        include/half.hpp
        main.cpp
)

