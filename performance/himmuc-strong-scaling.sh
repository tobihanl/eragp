#!/bin/bash
MEASURE_FILE=himmuc-strong-measure-1.dat
OUT_FILE=himmuc-strong-results-1.dat
PARAMS="-a -s3 -w10000 -h10000 -e3000,1000 -f0.08 -t3000"
RUNS=3
NODES=40

if [[ -f "$MEASURE_FILE" || -f "$OUT_FILE" ]]; then
    echo "$MEASURE_FILE or $OUT_FILE already exists"
    exit
fi

printf "Params: %s\n\n" "$PARAMS" >> $MEASURE_FILE
printf "Nodes Run Runtime\n" >> $MEASURE_FILE

for i in $(seq 1 $RUNS); do
  for n in $(seq -w 01 $NODES); do
    printf "%s %s " $n $i >> $MEASURE_FILE
    srun -p odr -N $n ./build/Evolution $PARAMS >> $MEASURE_FILE
  done
done

printf "Params: %s\n" "$PARAMS" >> $OUT_FILE
printf "Runs: %s\n" $RUNS >> $OUT_FILE
printf "Nodes: %s\n" $NODES >> $OUT_FILE
printf "\n" >> $OUT_FILE
printf "Nodes AverageRuntime\n" >> $OUT_FILE

awk -F " " -v nodes=$NODES -v runs=$RUNS 'NR>2 {a[$1]+=$3} END{for(i=1;i<=nodes;i++) print i, a[i]/runs}' $MEASURE_FILE >> $OUT_FILE
