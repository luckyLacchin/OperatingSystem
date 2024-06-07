.all: $(FILE)
	gcc SignalProxy.cc -o SignalProxy

$(FILE):
	@echo "start\n" > $(FILE)
#così viene generato automaticamente il file!

CLEAN: 
	rm $(FILE)
	rm SignalProxy

.PHONY:	all

#dovrebbe essere corretto, dovrei provarlo
