#!/bin/bash
MEASURE_FILE=measure-2.dat
OUT_FILE=results-2.dat
PARAMS="-m -r -a -s3 -w10000 -h10000 -e3000,1000 -f0.08 -t3000"
RUNS=3
THREADS=4

if [[ -f "$MEASURE_FILE" || -f "$OUT_FILE" ]]; then
    echo "$MEASURE_FILE or $OUT_FILE already exists"
    exit
fi

printf "Params: %s\n\n" "$PARAMS" >> $MEASURE_FILE
printf "Threads Run Runtime\n" >> $MEASURE_FILE

for i in $(seq 1 $RUNS); do
  for n in $(seq 1 $THREADS); do
    printf "%s %s " $n $i >> $MEASURE_FILE
    ./utils/run.sh ./build/Evolution $PARAMS -o$n >> $MEASURE_FILE
  done
done

scp mai08:~/evolution/eragp-maimuc-evo-2019/$MEASURE_FILE .

printf "Params: %s\n" "$PARAMS" >> $OUT_FILE
printf "Runs: %s\n" $RUNS >> $OUT_FILE
printf "Threads: %s\n" $THREADS >> $OUT_FILE
printf "\n" >> $OUT_FILE
printf "Threads AverageRuntime\n" >> $OUT_FILE

awk -F " " -v threads=$THREADS -v runs=$RUNS 'NR>2 {a[$1]+=$3} END{for(i=1;i<=threads;i++) print i, a[i]/runs}' $MEASURE_FILE >> $OUT_FILE
