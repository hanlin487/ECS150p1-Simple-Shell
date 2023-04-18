#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>

void handler(int signum){
    int c;
    read(STDIN_FILENO, &c, 1);
    fprintf(stderr, "%c\n", ++c);
}

int main(void){
    int c = 'a';
    int fd[2];
    struct sigaction sa = { 0 };

    pipe(fd);
    dup2(fd[0], STDIN_FILENO);
    dup2(fd[1], STDOUT_FILENO);

    sa.sa_handler = handler;
    sigaction(SIGUSR1, &sa, NULL);

    c++;
    write(STDOUT_FILENO, &c, 1);

    raise(SIGUSR1);
    
    return 0;
}