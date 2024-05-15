#!/usr/bin/env bash
#Scrivere uno script che dato un qualunque numero di argomenti li restituisca in  output in ordine inverso.

lista=()
while [[ $1 != "" ]]; do lista=( "$1" "${lista[@]}" ); shift; done
echo "${lista[@]}"

