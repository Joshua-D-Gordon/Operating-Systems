.PHONY: all
	
all: main

debug: main.c
	gcc -g main.c -o main -lm
main: main.c
	gcc main.c -o main -lm

clean:
	rm -f main