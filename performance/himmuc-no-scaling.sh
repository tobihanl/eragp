#!/bin/bash
MEASURE_FILE=himmuc-noscaling-measure-1.dat
OUT_FILE=himmuc-noscaling-results-1.dat
PARAMS="-a -s1 -t3000"
AREA_PER_PROCESS=10000000
LIVINGS_PER_PROCESS=2000
FOOD_PER_PROCESS=2000
RUNS=3
NODES=40

if [[ -f "$MEASURE_FILE" || -f "$OUT_FILE" ]]; then
    echo "$MEASURE_FILE or $OUT_FILE already exists"
    exit
fi

printf "# Params: %s\n" "$PARAMS" >> $MEASURE_FILE
printf "# Area per process: %s\n" $AREA_PER_PROCESS >> $MEASURE_FILE
printf "# Livings per process: %s\n" $LIVINGS_PER_PROCESS >> $MEASURE_FILE
printf "# Food per process: %s\n" $FOOD_PER_PROCESS >> $MEASURE_FILE
printf "# !!Actual number of nodes used was always 1!!\n\n" >> $MEASURE_FILE
printf "# SimulatedNodes Run Runtime\n" >> $MEASURE_FILE

for i in $(seq 1 $RUNS); do
  for n in $(seq 1 $NODES); do
    printf "%d %s " $n $i >> $MEASURE_FILE
    side=$(bc <<< "sqrt($AREA_PER_PROCESS * $n)")
    livings=$(($LIVINGS_PER_PROCESS * $n))
    food=$(($FOOD_PER_PROCESS * $n))
    srun -p odr -N 1 ./build/Evolution $PARAMS -w$side -h$side -e$livings,$food >> $MEASURE_FILE
  done
done

printf "# Params: %s\n" "$PARAMS" >> $OUT_FILE
printf "# Area per process: %s\n\n" $AREA_PER_PROCESS >> $OUT_FILE
printf "# Livings per process: %s\n\n" $LIVINGS_PER_PROCESS >> $OUT_FILE
printf "# Food per process: %s\n\n" $FOOD_PER_PROCESS >> $OUT_FILE
printf "# Runs: %s\n" $RUNS >> $OUT_FILE
printf "# Nodes: %s\n" $NODES >> $OUT_FILE
printf "\n" >> $OUT_FILE
printf "# Nodes AverageRuntime\n" >> $OUT_FILE

awk -F " " -v nodes=$NODES -v runs=$RUNS 'NR>5 {a[$1]+=$3} END{for(i=1;i<=nodes;i++) print i, a[i]/runs}' $MEASURE_FILE >> $OUT_FILE
