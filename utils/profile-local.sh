#!/bin/bash
hostname
hostname=$(hostname)
datafile="perf/perf.data.$hostname"
outfile="perf/perf.$hostname.svg"

perf record -o $datafile --call-graph dwarf ./build/Evolution $@
perf script -i $datafile | ./utils/stackcollapse-perf.pl | ./utils/flamegraph.pl > $outfile