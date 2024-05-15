#!/usr/bin/env bash
#Scrivere uno script che mostri il contenuto della cartella corrente in ordine inverso rispetto all’output generato da “ls” (che si può usare ma senza opzioni)


dati=( $(ls) ); lista=()
for i in ${!dati[@]}; do
  lista=( "${dati[$i]}" "${lista[@]}" )
done
echo "ORIGINAL:"; ls; echo; echo "REVERSED:"; echo "${lista[@]}"; echo

