#!/bin/bash
MEASURE_FILE=himmuc-timespent-measure-1.dat
LOG_FILE=himmuc-timespent-log
PARAMS="-a -s1 -w10000 -h10000 -e20000,20000 -t3000"
NODES=10

if [[ -f "$MEASURE_FILE" ]]; then
    echo "$MEASURE_FILE already exists"
    exit
fi

printf "# Params: %s\n\n" "$PARAMS" >> $MEASURE_FILE
printf "# Nodes Runtime\n" >> $MEASURE_FILE

for n in $(seq 1 $NODES); do
  printf "%s " $n >> $MEASURE_FILE
  srun -p odr -N 1 ./build/Evolution $PARAMS -l "$LOG_FILE-$n" >> $MEASURE_FILE
done
