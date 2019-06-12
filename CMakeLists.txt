cmake_minimum_required(VERSION 3.14)
project(Evolution)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake")

include_directories(include)

add_executable(${PROJECT_NAME} "")
add_subdirectory(src)