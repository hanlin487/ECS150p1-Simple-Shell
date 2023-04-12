#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <fcntl.h>

#define CMDLINE_MAX 512

//Output redirection, cd, and parsing all work

struct list {
    char** str_arr;
    char* file; //for output redirection
    int length;
};

//you cant use strlen on a double pointer so I added this struct to keep track of the array length

struct list* parse(char* string){
    struct list *l = malloc(sizeof(struct list));
    char temp[CMDLINE_MAX];

    
    l -> str_arr = (char**)malloc(16 * sizeof(char *));
    for (int i = 0; i < 16; i++){
	l -> str_arr[i] = (char*)malloc(32);
    }
    strcpy(temp,string);
    char* ptr;
    int c = 0;
    l -> file = (char*)malloc(32);
    ptr = strtok(temp, " ");
    while (ptr != NULL){
	//printf("%s\n",ptr);

	if (strcmp(ptr,">") == 0){
	    ptr = strtok(NULL, " ");
	    if (ptr != NULL){
		strcpy(l -> file, ptr);
		ptr = strtok(NULL," ");
		continue;
	    }else{
		l -> file = NULL;
		break;
	    }

	}
	
	strcpy(l -> str_arr[c],ptr);
	ptr = strtok(NULL," ");
	c += 1;
	
    }
    l -> length = c;
		




    
    return(l);
}

int getLength(struct list* l){
    return l -> length;
}

char** getList(struct list* l){
    return l -> str_arr;
}

char* getFile(struct list* l){
    return l -> file;
}



    
    

int main(){

	

        char cmd[CMDLINE_MAX];

        while (1) {
                char *nl;
                int retval;
		pid_t pid;

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

		

		/*
                retval = system(cmd);
                fprintf(stdout, "Return status value for '%s': %d\n",
                        cmd, retval);
		*/

		


		struct list* a = parse(cmd);
		char** ds = getList(a);
		char* cmd_2 = ds[0];
		int len = getLength(a);
		ds[len] = '\0';
		char* file = getFile(a);
		

		
		
		int fd;
		/*
		for (int i = 0; i < len; i++){
		    printf("%s\n",ds[i]);
		}
		*/
		
		//printf("%s\n",cmd_2);
		

		
		//execvp(cmd_2,ds);
		
		

		char buf[100];
		int ret;

		pid = fork();
		if (pid == 0){
		    //child

		    
		    if (strcmp(cmd_2,"cd") == 0){
			exit(0);
		    }

		    //point the stdout file descriptor to the redirection file 

		    if (file != NULL){
			fd = open(file,O_WRONLY | O_CREAT, 0644);
			dup2(fd, STDOUT_FILENO);
			close(fd);
		    }

		     	
			
		    ret = execvp(cmd_2,ds);
		    perror("Error: ");
		    exit(ret);
		    
		}else if (pid > 0){
		    if (strcmp(cmd_2,"cd") == 0){
			ret = chdir(ds[1]);
			printf("%s\n",getcwd(buf,100));
		    }
 
		    waitpid(pid, &retval,0);
		    fprintf(stderr, "Return status value for '%s': %d\n", cmd, WEXITSTATUS(retval));
		}
		else{
		    perror("Error:");
		    exit(1);
		    
		}
		
		   







		
        }

        return EXIT_SUCCESS;
}
