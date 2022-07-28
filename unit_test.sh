#!/bin/bash

mkdir -p result
cd result && rm -fr *
cd ..

for ((i=0;i<300;i++))
do
    ./build/linux/x86_64/release/lock_free > result/$i.txt
    if [ $? != 0 ];then
        echo "$i failed"
    else
        echo "$i success"
    fi
done
