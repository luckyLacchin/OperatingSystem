#let's start with makefile!

all: $(DIR)
	cp $(FILE) $(DIR)/$(FILE)
	gcc main.c -o $(DIR)/$(EXE)

$(DIR): $(FILE)
	mkdir $(DIR)

$(FILE):
	$(error File does not exist)

.PHONY: all