#!/bin/bash

traces=("fp_1.bz2" "fp_2.bz2" "int_1.bz2" "int_2.bz2" "mm_1.bz2" "mm_2.bz2")

# ghis_values=(11 12 13)
ghis_values=(13)

for trace in "${traces[@]}"
do
    echo "***** Running trace: $trace *****"
    echo
    
    for ghis in "${ghis_values[@]}"
    do
        echo "Running gshare with ghis=$ghis"
        bunzip2 -kc "../traces/$trace" | ./predictor --gshare:$ghis
        echo
    done
    
done