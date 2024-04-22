.PHONY: all
	
all: server client

server:  server.c
	gcc -o server server.c ../Base64Encode.c ../Base64Decode.c -lcrypto -lm

client:  client.c
	gcc -o client client.c ../Base64Encode.c ../Base64Decode.c -lcrypto -lm

.PHONY: clean

clean:
	-rm -f server client