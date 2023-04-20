#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>

int main(void){
    char cmd[] = "$a ls $c";
    char* ptr;
    char*temp;
    char copy[512];
    
    if ((ptr = strtok(cmd, "$"))){
        strcpy(copy, ptr);
        printf("$ ptr: %s\n", copy);
    }

    ptr = strtok(NULL, " ");
    strcpy(copy, ptr);
    printf("%s\n", copy);

    return 0;
}