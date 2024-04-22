.PHONY: all
	
all: client

client: client.c
	gcc -o client client.c ../Base64Encode.c ../Base64Decode.c -lcrypto -lm

.PHONY: clean

clean:
	-rm -f client