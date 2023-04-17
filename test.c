#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <fcntl.h>

int main(void){
    char* cmd = "pwd";
    char* args[] = {"pwd", NULL};
    execvp(cmd, args);
    
    return 0;
}