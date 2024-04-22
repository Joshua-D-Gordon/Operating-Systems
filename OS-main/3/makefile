.PHONY: all

all: encriptor decode encode

decode:	encriptor.h decode.c encriptor
	gcc decode.c -L. -l encriptor -o decode

encode:  encriptor.h encode.c encriptor
	gcc encode.c -L. -l encriptor -o encode

encriptor:	encriptor.h encriptor.c
	gcc -shared encriptor.c -fPIC -o libencriptor.so

.PHONY: clean

clean:
	-rm -f decode encode libencriptor.so 2>/dev/null