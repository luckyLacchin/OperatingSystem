#!/bin/bash
#questo dovrebbe essere l'unica cosa necessaria per shebang!

n=$#
if [[ $n != 10 ]]; then
    echo "Inserire dieci parole"
    exit 1
fi

args=("$@")

for ((i = 0; i < 10; i++)); do
    for ((j = i; j < 9 - i; j++)); do
        if [[ "${args[j]}" > "${args[j+1]}" ]]; then
            temp="${args[j+1]}"
            args[j+1]="${args[j]}"
            args[j]=$temp
        fi
    done
done

for ((i = 0; i < 10; i++)); do
    echo "${args[i]}"
done

echo "${args[@]}"

echo "${#args[@]}"

echo "${!args[@]}"