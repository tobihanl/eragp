#!/bin/bash
hostname
hostname=$(hostname)
datafile="perf/perf.data.$hostname"
outfile="perf/perf.$hostname.svg"

~/evolution/perf record -o $datafile --call-graph dwarf,1024 ./build/Evolution $@
~/evolution/perf script -i $datafile | ./utils/stackcollapse-perf.pl | ./utils/flamegraph.pl > $outfile
