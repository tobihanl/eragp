#!/bin/bash
TMP_FILE=tmp.out

if [[ -f "$TMP_FILE" ]]; then
    echo "$TMP_FILE already exists"
    exit
fi

if [[ "$#" -lt 1 ]]; then
    echo "Pass a glob pattern or list of files as argument"
    exit
fi

for i in $@; do
	awk -F "," '{for(i=4;i<=NF;i++)a[i]+=$i} END{print a[4],a[5],a[6],a[7],a[8],a[9],"\n"}' $i >> $TMP_FILE
done

echo "Sums: "
awk -F " " '{for(i=1;i<=NF;i++)a[i]+=$i} END{print "MPI\t"a[1]; print "Think\t"a[2]; print "Tick\t"a[3]; print "Render\t"a[4]; print "Delay\t"a[5]; print "Overall\t"a[6];}' $TMP_FILE

echo
echo "Percentages of Overall"
awk -F " " '{for(i=1;i<=NF;i++)a[i]+=$i} END{print "MPI\t"a[1]/a[6]; print "Think\t"a[2]/a[6]; print "Tick\t"a[3]/a[6]; print "Render\t"a[4]/a[6]; print "Delay\t"a[5]/a[6]; print "Overall\t"a[6]/a[6];}' $TMP_FILE

echo
echo "Percentages of Tick"
awk -F " " '{for(i=1;i<=NF;i++)a[i]+=$i} END{print "MPI\t"a[1]/a[3]; print "Think\t"a[2]/a[3]; print "Tick\t"a[3]/a[3];}' $TMP_FILE

rm $TMP_FILE

