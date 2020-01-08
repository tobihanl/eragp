#!/bin/bash
hostname
printenv PMI_RANK
rank=$(printenv PMI_RANK)
datafile="perf/perf.data.$rank"
outfile="perf/perf.$rank.svg"

perf record -o $datafile --call-graph dwarf,2048 ./build/Evolution $@
perf script -i $datafile | ./utils/stackcollapse-perf.pl | ./utils/flamegraph.pl > $outfile