// Base64.h

#ifndef BASE64_H
#define BASE64_H

#include <stdlib.h>

int Base64Decode(char* b64message, char** buffer);
int Base64Encode(const char* message, char** buffer);

#endif // BASE64_H