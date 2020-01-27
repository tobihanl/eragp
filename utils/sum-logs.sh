#!/bin/bash

if [[ "$#" -lt 1 ]]; then
    echo "Pass a glob pattern or list of files as argument"
    exit
fi

for i in $@; do
	echo $i
	awk -F "," '{for(i=4;i<=NF;i++)a[i]+=$i} END{print "MPI\t"a[4]; print "Think\t"a[5]; print "Tick\t"a[6]; print "Render\t"a[7]; print "Delay\t"a[8]; print "Overall\t"a[9];}' $i
	echo
done
