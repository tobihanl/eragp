#!/bin/bash
for i in 0 1 3 4 5 6 7 8 9
do
  echo "Copying to mai0$i"
  rsync -a --delete ./ mai0$i:~/evolution/eragp-maimuc-evo-2019
done
