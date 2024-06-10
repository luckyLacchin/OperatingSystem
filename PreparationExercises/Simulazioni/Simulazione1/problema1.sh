#let's start this agony....

read linecount

righe=1

if [[ -e $linecount ]]; then
    echo "The file exists"
    #adesso devo mostrare il numero di righe
    while read line; do
        ((++righe))
    done < $linecount
    echo $righe
else
    echo "The file doesn't exist"
fi

