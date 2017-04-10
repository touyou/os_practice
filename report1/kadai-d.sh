#!/bin/bash

find kadai-d -name "*.cpp" -print | while read name
do
    mv $name ${name/cpp/cc}   
    sed -i 's/NEET the 3rd/Fujii Yosuke/' ${name/cpp/cc}
    sed -i 's/neet3@example.com/touyu1121@is.s.u-tokyo.ac.jp/' ${name/cpp/cc}
    sed -i 's/ *$//' ${name/cpp/cc}     
done
