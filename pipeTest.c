#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <fcntl.h>

void piping(char* cmd1, char* args1[], char* cmd2, char* args2[]) {
    pid_t p1, p2;
    int fd[2];

    pipe(fd);

    if (!(p1 = fork())){
        close(fd[0]);
        dup2(fd[1], STDOUT_FILENO);
        close(fd[1]);
        execvp(cmd1, args1);
    }

    if (!(p2 = fork())){
        close(fd[1]);
        dup2(fd[0], STDIN_FILENO);
        close(fd[0]);
        execvp(cmd2, args2);
    }

    close(fd[0]);
    close(fd[1]);
    waitpid(p1, NULL, 0);
    waitpid(p2, NULL, 0);
}

int main(void) {
    char* c1 = "ls";
    char* arg1[] = {"ls", "-l", NULL};

    char* c2 = "wc";
    char* arg2[] = {"wc", "-l", NULL};
    
    //piping(c2, arg2, c1, arg1);
    piping(c1, arg1, c2, arg2);
    
    return 0;
}