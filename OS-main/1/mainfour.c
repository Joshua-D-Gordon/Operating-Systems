#include <stdio.h>

int main()
{
    int y;
    int *ptr = NULL;
    printf("enter a number\n");
    scanf("%d", &y);

    *ptr = y; // Dereferencing a null pointer, leading to a segmentation fault

    return 0;
}