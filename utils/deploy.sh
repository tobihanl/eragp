#!/bin/bash
for i in 0 1 2 3 4 5 6 7 8 9
do
  scp ./build/Evolution pi@mai0$i:~/evolution/eragp-maimuc-evo-2019/build/
  scp -r ./res pi@mai0$i:~/evolution/eragp-maimuc-evo-2019/
done