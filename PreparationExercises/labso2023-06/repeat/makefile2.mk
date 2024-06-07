#let's start with this one!

.all: $(DIR)
	cp $(FILE) $(DIR)/$(FILE)
	gcc main.c -o $(DIR)/$(EXE)

$(DIR): $(FILE)
	mkdir $(DIR)

$(FILE):
	$(error File does not exist)