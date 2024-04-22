#ifndef __CODECH__
#define __CODECH__

#define ABC "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890"

void* createCodec(char key[62]);

int encode(char* textin, char* textout, int len, void* codec);

int decode(char* textin, char* textout, int len, void* codec);

void freeCodec(void* codec);

typedef struct{
    char key[62];
} Codec;

#endif