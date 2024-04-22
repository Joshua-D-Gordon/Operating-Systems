.PHONY: all

all: proactor

proactor: proactor.c proactor.h
	gcc -c proactor.c -o proactor.o
	ar rcs libproactor.a proactor.o

.PHONY: clean

clean:
	-rm -f proactor.o libproactor.a