#let's start doing the makefile

BINARIO=coda

all: $(DEST)
	gcc main.c -o $(DEST)/$(BINARIO)

$(DEST):
	if [[ ! -d $(DEST) ]]; then
		$(error La cartella non esiste)
	fi

.PHONY: all