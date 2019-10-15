#!/bin/bash
echo "Init CMake:"
cd ./build/ || exit
cmake ./..
echo "Init CMake for debugging:"
cd ../debug/ || exit
cmake -DCMAKE_BUILD_TYPE=Debug ./..