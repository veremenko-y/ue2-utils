#!/bin/bash
cat 01main.s > out.s
cat 02lib.s  >> out.s

#TIME=strace -c
#TIME=time
TIME=

echo "Type 1"
$TIME $AS -o cat.obj out.s

echo "Type 2"
$TIME $AS -o merge.obj 01main.s 02lib.s