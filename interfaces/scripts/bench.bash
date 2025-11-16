#!/usr/bin/env bash

for file in bin/*; do
    printf '%s | ' "${file}"
    for _ in {1..25}; do
        ./a.out 4096 2160 45 2>&1 >/dev/null
    done | awk '{total+=$3;count++;array[NR]=$3} END {mean = total/count; for(i=1;i<=count;i++) {sumsq+=(array[i] - mean)^2}; variance = sqrt(sumsq/count); printf("Rendering stats mean: %f, stddev: %f, over %d runs\n", mean, variance, count)}'
done
