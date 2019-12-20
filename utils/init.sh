#!/bin/bash

if [[ $# -ne 2 ]]; then
  echo "You must specify exactly two parameters:"
  echo "1) build type (Debug, Release, RelWithDebInfo and MinSizeRel)"
  echo "2) render (ON/OFF"
else
  set -x
  cmake -DCMAKE_BUILD_TYPE=$1 -DDEFINE_RENDER=$2 -Bbuild -H.
fi

