#!/bin/bash

# Array to store runtimes
declare -a runtimes

# Compile your program if needed
# gcc -o your_program your_program.c

# Run the program 100 times
for ((i=1; i<=10; i++)); do
    # Record start time
    start=$(date +%s.%N)

    # Run the program, replace './your_program' with the actual command to run your program
    ./1BR

    # Record end time
    end=$(date +%s.%N)

    # Calculate runtime
    runtime=$(echo "$end - $start" | bc)

    # Store runtime in array
    runtimes[$i]=$runtime
done

# Calculate statistics
total=0
min=${runtimes[1]}
max=${runtimes[1]}
for ((i=1; i<=100; i++)); do
    total=$(echo "$total + ${runtimes[$i]}" | bc)
    if (( $(echo "${runtimes[$i]} < $min" | bc -l) )); then
        min=${runtimes[$i]}
    fi
    if (( $(echo "${runtimes[$i]} > $max" | bc -l) )); then
        max=${runtimes[$i]}
    fi
done
average=$(echo "scale=4; $total / 100" | bc)

# Print statistics
echo "Minimum Runtime: $min"
echo "Maximum Runtime: $max"
echo "Average Runtime: $average"
