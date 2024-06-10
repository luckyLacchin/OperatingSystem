#!/bin/bash

if [[ $# -eq 2 ]];
  then
    make
    ./app $1 $2
  else
    echo "Numero argomenti incoretto"
fi