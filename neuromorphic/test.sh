#!/bin/bash

iterations=0

# Loop indefinitely until '4/4' is found
while true; do
    # Increment the iteration count
    ((iterations++))

    # Run the command and check if it contains '4/4'
    if ./neuromorphic train_classification -d labeled_data/quadrents.json | grep -q "4/4"; then
        echo "Found 4/4 after $iterations iterations."
        break
    fi
done
