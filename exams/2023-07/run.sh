#!/bin/bash

# Check if the number of arguments is exactly 10
if [ "$#" -ne 10 ]; then
    echo "Error: Exactly 10 arguments are required."
    exit 1
fi

# Store the arguments in an array
arguments=("$@")

# Perform bubble sort to order the arguments alphabetically. Different sorting algorithms can be used.
for ((i = 0; i < 10; i++)); do
    for ((j = 0; j < 9 - i; j++)); do
        #This is a logical comparison that works for strings
        if [[ "${arguments[j]}" > "${arguments[j+1]}" ]]; then
            temp="${arguments[j]}"
            arguments[j]="${arguments[j+1]}"
            arguments[j+1]="$temp"
        fi
    done
done

# Output the sorted list
for argument in "${arguments[@]}"; do
    echo -n "$argument "
done
