#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    char * asd = "123";
    char * temp = (char*)malloc(sizeof(char) * (strlen("asd") + 1));
    strcpy(temp,asd);
    free(temp);
    printf("Pog");
    return 0;
}