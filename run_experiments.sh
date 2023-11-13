#!/bin/bash

# Define the pagers and tests
pagers=("apager" "dpager" "hpager" "hpagermore")
tests=("tests/single_access" "tests/seq_access" "tests/sparse_access" "tests/sparse_seq_access")

# Create a file to store results
results_file="results.txt"
echo "Pager,Test,Trial,Time,Memory" > $results_file

# Function to measure time and memory
measure() {
    /usr/bin/time -f "%e,%M" -o temp_time.txt --append --quiet ./$1 $2
}

# Run experiments
for pager in "${pagers[@]}"; do
    for test in "${tests[@]}"; do
        for trial in {1..10}; do
            echo "Running $pager with $test, trial $trial"
            measure $pager $test
            echo "$pager,$test,$trial,$(cat temp_time.txt)" >> $results_file
            > temp_time.txt  # Clear the temp file for the next measurement
        done
    done
done

# Call Python script to process results
python3 process_results.py $results_file
