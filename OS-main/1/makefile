.PHONY: all
	
all: main mainfour debug debugfour

debug: main.c
	gcc -g main.c -o mainDebug

debugfour: mainfour.c
	gcc -g mainfour.c -o mainfourDebug
	
mainfour: mainfour.c
	gcc mainfour.c -o mainfour
main: main.c
	gcc main.c -o main

clean:
	rm -f main mainfour mainDebug mainfourDebug