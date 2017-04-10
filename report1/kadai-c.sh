#!/bin/sh


mkdir temp

for i in `seq -w 0 1 99`
do
    wget -O "temp/1.pdf.$i" "http://pf.is.s.u-tokyo.ac.jp/jp/class/syspro/kadai1/1.pdf.$i"
    cat temp/1.pdf.$i >> 1.pdf
done

rm -rf temp
