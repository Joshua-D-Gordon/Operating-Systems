.PHONY: all

all: main

main: main.c ../2/proactor.h
	gcc main.c -o main -L. ../2/libproactor.a

.PHONY: clean

clean:
	-rm -f main