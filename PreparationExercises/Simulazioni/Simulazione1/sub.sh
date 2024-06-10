#eddaje

n=($1) #così ottengo il primo numero
counter=0

while [[ $counter < $n ]]; do
    echo $( echo $BASHPID )
    echo "generato sottoprocesso"
    ((++counter))
done;

echo $HOME

echo $(pwd)