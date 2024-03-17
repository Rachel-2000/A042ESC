#!/bin/bash

traces=("fp_1.bz2" "fp_2.bz2" "int_1.bz2" "int_2.bz2" "mm_1.bz2" "mm_2.bz2")

ghis_values=(9)  
lhis_values=(10) 
pc_index_values=(6 8 10 12)

for trace in "${traces[@]}"
do
    echo "***** Running trace: $trace *****"
    echo
    
    for ghis in "${ghis_values[@]}"
    do
        for lhis in "${lhis_values[@]}"
        do
            for pc_index in "${pc_index_values[@]}"
            do
                echo "Running tournament with ghis=$ghis, lhis=$lhis, pc_index=$pc_index"
                bunzip2 -kc "../traces/$trace" | ./predictor --tournament:$ghis:$lhis:$pc_index
                echo

                # use for comparison
                # echo "Running custom with ghis=$ghis, lhis=$lhis, pc_index=$pc_index"
                # bunzip2 -kc "../traces/$trace" | ./predictor --custom:$ghis:$lhis:$pc_index
                # echo
            done
        done
    done
    
done
