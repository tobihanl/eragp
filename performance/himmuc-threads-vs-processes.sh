#!/bin/bash
MEASURE_FILE=himmuc-threads-processes-measure-1.dat
OUT_FILE=himmuc-threads-processes-results-1.dat
PARAMS="-a -s3 -w10000 -h10000 -e3000,1000 -f0.08 -t3000"
RUNS=3

if [[ -f "$MEASURE_FILE" || -f "$OUT_FILE" ]]; then
    echo "$MEASURE_FILE or $OUT_FILE already exists"
    exit
fi

printf "# Params: %s\n\n" "$PARAMS" >> $MEASURE_FILE
printf "# Processes Threads Run Runtime\n" >> $MEASURE_FILE

for i in $(seq 1 $RUNS); do
  printf "1 4 %s " $i >> $MEASURE_FILE
  srun -p odr -N 1 -n1 ./build/Evolution $PARAMS -o4 >> $MEASURE_FILE
  printf "2 2 %s " $i >> $MEASURE_FILE
  srun -p odr -N 1 -n2 ./build/Evolution $PARAMS -o2 >> $MEASURE_FILE
  printf "4 1 %s " $i >> $MEASURE_FILE
  srun -p odr -N 1 -n4 ./build/Evolution $PARAMS -o1 >> $MEASURE_FILE
done

printf "# Params: %s\n" "$PARAMS" >> $OUT_FILE
printf "# Runs: %s\n" $RUNS >> $OUT_FILE
printf "\n" >> $OUT_FILE
printf "# Processes Threads AverageRuntime\n" >> $OUT_FILE

awk -F " " -v runs=$RUNS 'NR>2 {a[$1]+=$4} END{print "1 4", a[1]/runs; print "2 2", a[2]/runs; print "4 1", a[4]/runs}' $MEASURE_FILE >> $OUT_FILE
