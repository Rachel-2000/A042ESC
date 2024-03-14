#!/bin/bash

traces=("fp_1.bz2" "fp_2.bz2" "int_1.bz2" "int_2.bz2" "mm_1.bz2" "mm_2.bz2")

predictors=("static" "gshare:13" "tournament:9:10:10" "custom:13:13:10")

for trace in "${traces[@]}"
do
    echo "***** Running trace: $trace *****"
    echo
    
    for predictor in "${predictors[@]}"
    do
        echo "Running predictor: $predictor"
        bunzip2 -kc "../traces/$trace" | ./predictor --"$predictor"
        echo
    done
done