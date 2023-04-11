#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define CMDLINE_MAX 512

int main(void)
{
        char cmd[CMDLINE_MAX];

        while (1) {
                char *nl;
                int retval;

                /* Print prompt */
                printf("sshell$ ");
                fflush(stdout);

                /* Get command line */
                fgets(cmd, CMDLINE_MAX, stdin);

                /* Print command line if stdin is not provided by terminal */
                if (!isatty(STDIN_FILENO)) {
                        printf("%s", cmd);
                        fflush(stdout);
                }

                /* Remove trailing newline from command line */
                nl = strchr(cmd, '\n');
                if (nl)
                        *nl = '\0';

                /* Builtin command */
                if (!strcmp(cmd, "exit")) {
                        fprintf(stderr, "Bye...\n");
                        break;
                }

                /* Regular command */
                /*retval = system(cmd);
                fprintf(stdout, "Return status value for '%s': %d\n",
                        cmd, retval);
                */
                
                /* phase 1 parent-child exec */
                pid_t pid;
                char* args[] = {cmd, NULL};
                pid = fork();

                if (pid == 0){          // CHILD PROCESS
                        execvp(cmd, args);
                        perror("execvp");
                        exit(1);
                }
                else if (pid > 0){      // PARENT PROCESS
                        waitpid(pid, &retval, 0);
                        fprintf(stderr, "Return status value for '%s': %d\n",
                                cmd, WEXITSTATUS(retval));
                }
                else {
                        perror("fucked fork");
                        exit(1);
                }
                printf("\n");
        }
        return EXIT_SUCCESS;
}
