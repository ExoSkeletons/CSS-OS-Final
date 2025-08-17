cmake_minimum_required(VERSION 3.30.5)
project(Final)

set(CMAKE_CXX_STANDARD 20)

add_executable(Final main.cpp
        LF.cpp
        LF.hpp
        Pipeline.cpp
        Pipeline.hpp
        PthTools.hpp)
