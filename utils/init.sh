#!/bin/bash

echo "Init CMake:"
if [[ "$1" == "-build" ]]; then
    cd ./build/ || exit

    if [[ "$2" == "-render" ]]; then
      echo "Parameters: build and render"
      cmake -DDEFINE_RENDER=ON ./..
    else
      echo "Parameters: build"
      cmake ./..
    fi

elif [[ "$1" == "-debug" ]]; then
    cd ./debug/ || exit

    if [[ "$2" == "-render" ]]; then
      echo "Parameters: debug and render"
      cmake -DCMAKE_BUILD_TYPE=Debug -DDEFINE_RENDER=ON ./..
    else
      echo "Parameters: debug"
      cmake -DCMAKE_BUILD_TYPE=Debug ./..
    fi
else
    echo "First parameter must be '-build' or '-debug'"
fi
