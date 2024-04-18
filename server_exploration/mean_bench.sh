#!/bin/bash

# Array of values
values=(16 256 512 1024 2048 4096 8192)

# Function to calculate mean
mean() {
    local sum=0
    for val in "$@"; do
        sum=$((sum + val))
    done
    echo "$sum / $#" | bc -l
}

# Function to calculate median
median() {
    local arr=($(printf '%s\n' "$@" | sort -n))
    local count=${#arr[@]}
    local mid=$((count / 2))
    if ((count % 2 == 0)); then
        echo "(${arr[mid - 1]} + ${arr[mid]}) / 2" | bc -l
    else
        echo "${arr[mid]}"
    fi
}

# Loop through each value
for val in "${values[@]}"
do
    # Array to store results
    results=()

    # Repeat each test 3 times
    for ((i=1; i<=3; i++))
    do
        # Run the command with the current value and capture the last 2 lines of stdout
        output=$(wrk -t 6 -c $val -d 30s http://localhost:8069/http_uring_rough.c | tail -n 2)

        # Extract the result and append it to the results array
        result=$(echo "$output" | tail -n 1 | awk '{print $2}')
        results+=("$result")
    done

    # Calculate mean and median
    mean_value=$(mean "${results[@]}")
    median_value=$(median "${results[@]}")

    # Print results
    echo "Value $val:"
    echo "Mean: $mean_value"
    echo "Median: $median_value"
done
