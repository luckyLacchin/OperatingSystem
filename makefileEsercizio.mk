# Creare un makefile con una regola help di default che mostri una nota informativa, 
# una regola backup che crei un backup di una cartella appendendo “.bak” al nome 
# e una restore che ripristini il contenuto originale. Per definire la cartella 
# sorgente passarne il nome come variabile, ad esempio:
# make -f mf-backup FOLDER=... 



# Defining the SHELL variable will change the shell used for the recipies
SHELL := /bin/bash
# Default value of folder
FOLDER := '/tmp'

help:
	@echo "make -f mf-backup backup FOLDER="/code""
	@echo "make -f mf-backup restore FOLDER="/lol""









backup: $(FOLDER)
	@echo "Backup of folder $(FOLDER) as $(FOLDER).bak..." ; sleep 1s
	@[[ -d $(FOLDER).bak ]] && echo "?Error" || cp -r $(FOLDER) $(FOLDER).bak









restore: $(FOLDER).bak
	@echo "Restore of folder  $(FOLDER) from $(FOLDER).bak..." ; sleep 1s
	@[[ ! -d $(FOLDER).bak ]] && echo "?Error" || rm -rf $(FOLDER) && cp -r $(FOLDER).bak $(FOLDER)

error:
	$(error This is a makefile error!)

.PHONY: help backup restore