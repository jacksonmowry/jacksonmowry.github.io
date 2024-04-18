#!/bin/bash

calculate_average() {
    # Initialize variables
    local sum=0
    local count=0

    # Loop through the array elements
    for value in "${@}"; do
        # Add each value to the sum
        sum=$(echo "$sum + $value" | bc)
        ((count++))
    done

    # Calculate the average
    if [ "$count" -gt 0 ]; then
        average=$(echo "scale=2; $sum / $count" | bc)
        echo "$average"
    else
        echo "Error: No values provided."
        return 1
    fi
}

# Array of values
values=(1 4 8 16 32 64 128 256 512 1024 2048 4096 8192)

printf "| %-4s | %-8s | %-8s | %-6s |\n" "Conn" "Req/sec" "Mb/sec" "P99"

# Loop through each value
for val in "${values[@]}"
do

    latency_array=()
    requests_array=()
    mbps_array=()


    for ((i=1; i<=3; i++))
    do
    # Run the command with the current value and capture the last 2 lines of stdout
    if [ $val -ge 8 ]; then
        output=$(wrk -t 6 -c $val -d 30s --latency http://localhost:8069/http_uring_rough.c | grep -v "Socket errors:" | tail -n 4)
    else
        output=$(wrk -t $val -c $val -d 30s --latency http://localhost:8069/http_uring_rough.c | tail -n 4)
    fi

    latency_array+=("$(echo $output | awk '{print $2}')")
    requests_array+=("$(echo $output | tail -n 2 | head -n 1| awk '{print $10}')")
    mbps_array+=("$(echo $output | tail -n 1 | awk '{print $12}' | sed 's/MB//')")


    # Prepend the value to the output
    # printf "%d:\n%s\n" "$val" "$output"
    done

    req_avg=$(calculate_average "${requests_array[@]}")
    mbps_avg=$(calculate_average "${mbps_array[@]}")

    printf "| %-4d | %-8s | %-8s | %-8s | %-8s %-8s %-8s |\n" "$val" "$req_avg" "$mbps_avg" "${latency_array[1]}" "${latency_array[0]}" "${latency_array[1]}" "${latency_array[2]}"

done
