#let's start with makefile!

all: $(DIR)
	cp $(FILE) $(DIR)$(FILE)
	gcc main.c -o $(DIR)/$(EXE)

$(DIR): $(FILE)
	if [[ ! -d $(DIR) ]]; then
		mkdir $(DIR)
	fi

$(FILE):
	if [[ ! -e $(FILE) ]]; then
		$(error File does not exist)
	fi

.PHONY: all