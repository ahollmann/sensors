cmake_minimum_required(VERSION 3.19)

project(sensors)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(sensors main.cpp Sensor.cpp)
target_include_directories(sensors SYSTEM PRIVATE third_party)