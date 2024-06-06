if [ $# -ne 1 ]; then
    echo "Inserire il nome di un file"
    exit 1
fi

filename = "$1"

# Verifica che il file esista
if [ ! -f "$filename" ]; then
    echo "Error: File '$filename' not found"
    exit 1
fi

counter=0

while read line; do
    if ((counter%2==0)); then
        echo "$line" >&1
    else
        echo "$line" >&2
    ((++counter))
    fi
done < "$filename"