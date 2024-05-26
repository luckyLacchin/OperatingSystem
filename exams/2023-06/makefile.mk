#This is the all rule that will be executed by default. It has one prerequisite: the DIR variable.
#If DIR does not exist, then we will look for the rule associated to it.
all: $(DIR) 
	
	cp $(FILE) $(DIR)/$(FILE)
	gcc main.c -o $(DIR)/$(EXE)

# This rule will be executed only if the DIR does not exist. It has one prerequisite: the FILE variable.
$(DIR): $(FILE)
	mkdir $(DIR)

# This rule should create the file, but it simply exits with an error. If we enter in this rule
# it means that the file does not exist.
$(FILE):
	$(error File does not exist)
