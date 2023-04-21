#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/stat.h>

#define CMDLINE_MAX 512
#define BUF_MAX 100
#define FILE_LEN 16
#define CMD_ARR_LEN 16
#define CMD_LEN 32

struct node {
    char* file; 			//File for output redirect (check parse function to see how its set
    char** command; 		//String array representing the command (for execvp)
    struct node* next;
    int exit; 
    int length; 			//length of string array
};

//make empty node
struct node* createNode(void){
    struct node *n = malloc(sizeof(struct node));
    n -> file = (char*) malloc(FILE_LEN);
    n -> command = (char**) malloc(CMD_ARR_LEN * sizeof(char*));
   
	for (int i = 0; i < CMD_ARR_LEN; i++){
		n -> command[i] = (char*) malloc(CMD_LEN);
    }

    n -> file = (char*) malloc(FILE_LEN);
    n -> next = NULL;
    n -> length = 0;
    n -> exit = 0;
    return(n);
}

int getLength(struct node* n){
    return n -> length;
}

char** getCommand(struct node* n){
    return n -> command;
}

char* getFile(struct node* n){
    return n -> file;
}

int getExit(struct node* n){
    return n -> exit;
}

struct list {
    struct node* head;
    struct node* tail;
    struct node* curr;
    int length;
};

//make empty list
struct list* createList(void){
    struct list *l = malloc(sizeof(struct list));
    l -> tail = NULL;
    l -> head = NULL;
    l -> curr = NULL;
    l -> length = 0;
	
    return(l);
}

void insert(struct list* l, struct node* n){
	if (l -> tail == NULL){
		l -> tail = n;
		l -> head = n;
		n -> next = NULL;
		l -> length += 1;

		return;
    }

    l -> tail -> next = n;
    n -> next = NULL;
    l -> tail = n;
    l -> length +=1;
}

void front(struct list* l){
	if (l -> head == NULL){
		return;
    }

    l -> curr = l -> head;
}

void right(struct list* l){
	if (l -> curr == NULL){
		return;
    }

    l -> curr = l -> curr -> next;
}

struct node* view(struct list* l){
    return l -> curr;
}

int getLen(struct list* l){
    return l -> length;
}

void parse(struct node* n, char* string, char** env_vars){
    char temp[CMDLINE_MAX];
    strcpy(temp, string);

    char* ptr;
    char ptr2[CMD_LEN];
    int c = 0;
    ptr = strtok(temp, " ");
    
	while (ptr != NULL){
		if (c >= CMD_ARR_LEN){
		    fprintf(stderr,"Error: too many process arguments\n");
		    break;
		}

		//if output redirection used then set file variable of node object to 
		//output destination
		if (ptr[0] == '$'){
		    strcpy(ptr2,ptr+1);

		    if (ptr2 != NULL){

				if (strlen(ptr2) == 1 && islower(*ptr2)){
					strcpy(n -> command[c], env_vars[*ptr2 - 'a']);
					ptr = strtok(NULL, " ");
					c += 1;
					continue;
				}
				else{
					fprintf(stderr,"Error: invalid variable name");
					exit(1);
				}
			}
		}

		//add one more if for case where its like world>file
		if (strcmp(ptr, ">") == 0){
			ptr = strtok(NULL, " ");

			if (ptr != NULL){
			    	int temp;
				temp = open(ptr, O_WRONLY | O_CREAT, 0644);
			
				if (temp == -1){
				    fprintf(stderr,"Error: cannot open output file");
				    exit(1);
				}

				close(temp);
				strcpy(n -> file, ptr);
				ptr = strtok(NULL, " ");
				continue;
			}
			else{
				n -> file = NULL;
				fprintf(stderr,"Error: no output file\n");
				exit(1);
			}
		}
		strcpy(n -> command[c],ptr);
		ptr = strtok(NULL," ");
		c += 1;
    }
    n -> length = c;
    n -> command[c] = '\0';
}

void pipeline(struct list* l){
    int length = getLen(l);
    int status;
    int children[CMD_ARR_LEN];
    int fd[2];
    int prev;
    int output;
    char file[FILE_LEN];
    char** command;
    //pid_t p1;

    prev = STDIN_FILENO;
    front(l);
    

    for (int i = 0; i < length; i++){
		pipe(fd);
		command = getCommand(view(l));

		//children[i] = fork();

		//exit if command is cd or pwd
		if (strcmp(command[0], "cd") == 0){
			exit(0);
		}

		if (strcmp(command[0], "pwd") == 0){
			exit(0);
		}

		if (strcmp(command[0], "set") == 0){
		        exit(0);
		}
		children[i] = fork();

		if (children[i] == 0){
			if (prev != STDIN_FILENO){
				dup2(prev, STDIN_FILENO);
				close(prev);
			}

			if (i != length - 1){
				dup2(fd[1], STDOUT_FILENO);
				close(fd[1]);
			}
			
			close(fd[0]);
			
			if (getFile(view(l)) != NULL){
				strcpy(file, getFile(view(l)));
				output = open(file, O_WRONLY | O_CREAT, 0644);
				

				dup2(output, STDOUT_FILENO);
				

				close(output);
			}	

			execvp(command[0],command);
			fprintf(stderr,"Error: command not found\n");

			exit(1);
		}

		if (prev != STDIN_FILENO){
			close(prev);
		}

		close(fd[1]);
		prev = fd[0];
		right(l);
    }
		for (int i =0; i <length;i++){
		    waitpid(children[i],&status,0);
		    //REPLACE CURRENT VALS WITH WEXITSTATUS AND RETURN THE ARRAY/PRINT IN MAIN
		    // printf("%d\n",WEXITSTATUS(status));
		}
		exit(0);
}

int main(void){
	char cmd[CMDLINE_MAX]; 
        char* env_vars[26];
        int ev_index;

        for (int i = 0; i < 26; i++){
                env_vars[i] = "";
        }
	while (1) {
	 
		char buf[BUF_MAX];
		char *nl;
		char** new_cmd;
		int retval;
		pid_t pid;

		//Print prompt 
		printf("sshell$ ");
		fflush(stdout);

		//Get command line 
		fgets(cmd, CMDLINE_MAX, stdin);

		//Print command line if stdin is not provided by terminal 
		if (!isatty(STDIN_FILENO)) {
			printf("%s", cmd);
			fflush(stdout);
		}

		//Remove trailing newline from command line 
		nl = strchr(cmd, '\n');

		if (nl)
			*nl = '\0';

		//Builtin command 
		if (!strcmp(cmd, "exit")) {
			fprintf(stderr, "Bye...\n");
			fprintf(stderr, "+ completed 'exit' [0]\n");
			break;
		}
		
		char copy_temp[CMDLINE_MAX];
		char** store_commands = (char**) malloc(CMD_ARR_LEN * sizeof(char*));
		char* ptr;
		int c = 0;
		
		for (int i = 0; i < 16; i++){
			store_commands[i] = (char*) malloc(CMD_LEN);
		}

		strcpy(copy_temp,cmd);
		ptr = strtok(copy_temp,"|");

		while (ptr != NULL){
		        //printf("%s\n",ptr);
			strcpy(store_commands[c], ptr);
			ptr = strtok(NULL,"|");
			c+=1;
		}

		struct list* a = createList();
		struct node* n;

		for (int i = 0; i < c; i++){
			n = createNode();
			parse(n, store_commands[i],env_vars);
			insert(a, n);
		}

		if (getLen(a) == 0){
		    continue;
		}
		front(a);
		new_cmd = getCommand(view(a));
  
		//fork to start the shell process executions
		pid = fork();

		//CHILD PROCESS
		if (pid == 0){  
			pipeline(a);
		}
		else if (pid > 0){

		    	int temp;
			bool built = false;

			//built-in commands
			if (strcmp(new_cmd[0], "cd") == 0){
			        built = true;
			        if (chdir(new_cmd[1]) != 0){
				    fprintf(stderr,"Error: cannot cd into directory\n");
				    temp = 1;
				}
				else{
				    temp = 0;
				}

				printf("%s\n", getcwd(buf, BUF_MAX));
			}
			else if (strcmp(new_cmd[0], "pwd") == 0){
				built = true;
				temp = 0;
				printf("%s\n", getcwd(buf, BUF_MAX));
			}

			//add errors here
        	else if (strcmp(new_cmd[0], "set") == 0){
				ev_index = *new_cmd[1] - 'a';
				env_vars[ev_index] = new_cmd[2];
			}
			
			waitpid(pid, &retval,0);
			if (built){
			    fprintf(stderr, "+ completed '%s' [%d]\n",cmd,temp);
			}else{
			    fprintf(stderr, "+ completed '%s' [%d]\n", cmd, temp);
			}
		}
		else{
			perror("Error:");
			exit(1);
		}
	}
    return EXIT_SUCCESS;
}

