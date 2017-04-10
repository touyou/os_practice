#!/bin/sh

find coreutils-8.9 -name "*.c" -print | while read line
do
    wc -l $line
done |
sort | while read line
do
    set -- $line
    echo $2 >> result.txt
done
