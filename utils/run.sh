#!/bin/bash
mpirun -np 10 -hosts mai08,mai09,mai06,mai07,mai04,mai05,mai02,mai03,mai00,mai01 -env DISPLAY=:0 $@
