#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "encriptor.h"
#include <ctype.h>

void *createCodec(char key[62])
{
    // Check for invalid key length (not 62 chars)
    if(strlen(key)!= 62){
        printf("key not eql to 62 for codec\n");
        return NULL;
    }
    //allocate memory for Codec
    Codec *globalCodec = (Codec *)malloc(sizeof(Codec));
    // Check if memory was allocated
    if (globalCodec == NULL)
    {   
        printf("global Codec not created in malloc function\n");
        return NULL;
    }

    // Check if all the charaters exist and that there are only one existence for each character
    //just one long array of 127 all values from a - 0
    int allCharacter[127] = {0}; // array of all 0's - we will put 1 where a char is found
    
    for (int i = 0; i < 62; i++)
    {
        int currentPos = key[i];

        if (allCharacter[currentPos] == 1) // OPS already exists!
        {   
            printf("global codec charaters exist and that there are only one existence for each character ERROR\n");
            //error found free codec and exit
            free(globalCodec);
            return NULL;
        }
        //put 1 where char is found
        allCharacter[currentPos] = 1;
    }
    //copy to struct in key value the key
    strcpy(globalCodec->key, key);
    //return the Codec
    return globalCodec;
}

int encode(char *textin, char *textout, int len, void *codec)
{
    //if no input file or no output file or no Codec return -1 (flag error)
    if (textin == NULL || textout == NULL || codec == NULL)
    {
        return -1;
    }
    //difne local myCodec from void codec given in function
    Codec *myCodec = (Codec *)codec;
    //countvalue for return return
    int count = 0;
    for (int i = 0; i < len; i++)
    {
        char current = textin[i];
        int index = -1;
        //find index in ABC
        for (int j = 0; j < 62; j++)
        {
            if (ABC[j] == current)
            {
                index = j;
                break;
            }
        }
        if (index != -1)
        //put in textout the value in key at that index
        {
            count = count + 1;
            textout[i] = myCodec->key[index];
        }
    }
    //return count
    return count;
}

int decode(char *textin, char *textout, int len, void *codec)
{
    //if no input file or no output file or no Codec return -1 (flag error)
    if (textin == NULL || textout == NULL || codec == NULL)
    {
        return -1;
    }
    //difne local myCodec from void codec given in function
    Codec *myCodec = (Codec *)codec;
    //countvalue for return return
    int count = 0;

    for (int i = 0; i < len; i++)
    {
        char current = textin[i];
        int index = -1;

        //find index in key
        for (int j = 0; j < 62; j++)
        {
            if (myCodec->key[j] == current)
            {
                index = j;
                break;
            }
        }
        //put in textout the value in ABC at that index
        if (index != -1)
        {
            textout[i] = ABC[index];
            count = count + 1;
        }
    }
    //return count
    return count;
}

//free codec if not already NULL
void freeCodec(void *codec)
{
    if (codec == NULL)
    {
        fprintf(stderr, "Error: freeCodec called without codec initialization.\n");
        return;
    }

    free(codec);
}
