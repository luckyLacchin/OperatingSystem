default: app.out

app.out: main.c
	@gcc -o app main.c

clean:
	mkdir -p build
	rm -rf build *.out *.s

.PHONY: clean default app.out

.SECONDARY: make.s
