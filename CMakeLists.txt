cmake_minimum_required(VERSION 3.5)
project(Evolution)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake")

find_package(MPI REQUIRED)

include_directories(${MPI_INCLUDE_PATH})
include_directories(include)

add_executable(${PROJECT_NAME} "")
include(src/CMakeLists.txt)
include(src/SimplexNoise/CMakeLists.txt)

# Adding MPI to build target
target_link_libraries(${PROJECT_NAME} PUBLIC ${MPI_LIBRARIES})

if (MPI_COMPILE_FLAGS)
    set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "${MPI_COMPILE_FLAGS}")
endif ()

if (MPI_LINK_FLAGS)
    set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "${MPI_LINK_FLAGS}")
endif ()

OPTION(DEFINE_RENDER "Compile with rendering" OFF) # Disabled by default

if (DEFINE_RENDER)
    ADD_DEFINITIONS(-DRENDER)
endif ()

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer")
SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fno-omit-frame-pointer")

find_package(OpenMP)
target_link_libraries(${PROJECT_NAME} PRIVATE "${OpenMP_CXX_FLAGS}")
target_compile_options(${PROJECT_NAME} PRIVATE "${OpenMP_CXX_FLAGS}")
