#!/bin/bash
hostname
source /etc/profile.d/modules.sh
module load perf

perf record -o perf/perf.data.$(hostname) -g ./debug/Evolution $@
perf script -i perf/perf.data.$(hostname) | ./utils/stackcollapse-perf.pl | ./utils/flamegraph.pl > perf/perf.$(hostname).svg
