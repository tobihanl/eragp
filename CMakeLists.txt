cmake_minimum_required(VERSION 3.14)
project(Evolution)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake")

find_package(SDL2 REQUIRED)

include_directories(${SDL2_INCLUDE_DIR})

add_executable(Evolution Main.cpp)
target_link_libraries(${CMAKE_PROJECT_NAME} ${SDL2_LIBRARY})