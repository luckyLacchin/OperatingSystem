#!/bin/bash
#questo line precedente serve per usare shebang

if [ $# -ne 10 ]; then
    echo "The number of words given is not 10"
    exit 1
fi

args=("$@")

for ((i = 0; i < 10; i++)); do
    for ((j = 0; j < 9 - i; j++)); do
        if [[ "${args[j]}" > "${args[j+1]}" ]]; then
            temp="${args[j+1]}"
            args[j+1]="${args[j]}"
            args[j]="$temp"
        fi
    done
done

echo ${args[@]}