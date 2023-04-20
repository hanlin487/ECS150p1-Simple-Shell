#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <fcntl.h>

#define CMDLINE_MAX 512
#define BUF_MAX 100
#define FILE_LEN 16
#define CMD_ARR_LEN 16
#define CMD_LEN 32

struct node {
    char* file; 			//File for output redirect (check parse function to see how its set
    char** command; 		//String array representing the command (for execvp)
    struct node* next; 
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

void parse(struct node* n, char* string){
    char temp[CMDLINE_MAX];

	n -> command = (char**) malloc(CMD_ARR_LEN * sizeof(char *));

    for (int i = 0; i < 16; i++){
		n -> command[i] = (char*) malloc(CMD_LEN);
    }

    strcpy(temp, string);

    char* ptr;
    int c = 0;
    ptr = strtok(temp, " ");
    
	while (ptr != NULL){

		//if output redirection used then set file variable of node object to 
		//output destination
		if (strcmp(ptr, ">") == 0){
			ptr = strtok(NULL, " ");

			if (ptr != NULL){
				strcpy(n -> file, ptr);
				ptr = strtok(NULL, " ");
				continue;
			}
			else{
				n -> file = NULL;
				break;
			}
		}
		strcpy(n -> command[c], ptr);
		ptr = strtok(NULL," ");
		c += 1;
    }
    n -> length = c;
    n -> command[c] = '\0';
}

void pipeline(struct list* l){
    int length = getLen(l);
    int fd[2];
    int prev;
    int output;
    char file[FILE_LEN];
    char** command;
    pid_t p1;

    prev = STDIN_FILENO;
    front(l);

    for (int i = 0; i < length; i++){
		pipe(fd);
		command = getCommand(view(l));

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

		p1 = fork();

		if (p1 == 0){
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
			//exit(0);
		}

		if (prev != STDIN_FILENO){
			close(prev);
		}

		close(fd[1]);
		prev = fd[0];
		right(l);
    }
    exit(waitpid(p1, NULL, 0));
}

int main(void){
	char cmd[CMDLINE_MAX];
	char* env_vars[26];
	int ev_index;

	for (int i = 0; i < 26; i++){
		env_vars[i] = "";
		printf("env[%d]: %s\n", i, env_vars[i]);
	}

	while (1) {
		char buf[BUF_MAX];
		char *nl;
		char** new_cmd;
		int retval;
		pid_t pid;
		struct node* new = createNode();

		//Print prompt 
		printf("\nsshell$ ");
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
			break;
		}

		//parse the command line string into the node command object
		parse(new, cmd);
		new_cmd = getCommand(new);
		printf("command: %s\n", new_cmd[0]);

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
			strcpy(store_commands[c], ptr);
			ptr = strtok(NULL,"|");
			c+=1;
		}

		struct list* a = createList();
		struct node* n;

		for (int i = 0; i < c; i++){
			n = createNode();
			parse(n, store_commands[i]);
			insert(a, n);
		}

		//fork to start the shell process executions
		pid = fork();

		//CHILD PROCESS
		if (pid == 0){
			// char copy_temp[CMDLINE_MAX];
			// char** store_commands = (char**) malloc(CMD_ARR_LEN * sizeof(char*));
			// char* ptr;
			// int c = 0;
			
			// for (int i = 0; i < 16; i++){
			// 	store_commands[i] = (char*) malloc(CMD_LEN);
			// }

			// strcpy(copy_temp,cmd);
			// ptr = strtok(copy_temp,"|");

			// while (ptr != NULL){
			// 	strcpy(store_commands[c], ptr);
			// 	ptr = strtok(NULL,"|");
			// 	c+=1;
			// }

			// struct list* a = createList();
			// struct node* n;

			// for (int i = 0; i < c; i++){
			// 	n = createNode();
			// 	parse(n, store_commands[i]);
			// 	insert(a, n);
			// }

			pipeline(a);
		}
		else if (pid > 0){

			//built-in commands
			if (strcmp(new_cmd[0], "cd") == 0){
				chdir(new_cmd[1]);
				printf("%s\n", getcwd(buf, BUF_MAX));
			}
			else if (strcmp(new_cmd[0], "pwd") == 0){
				printf("%s\n", getcwd(buf, BUF_MAX));
			}
			else if (strcmp(new_cmd[0], "set") == 0){
				ev_index = *new_cmd[1] - 'a';
				env_vars[ev_index] = new_cmd[2];
				//printf("stored: %s\n", env_vars[ev_index]);
			}
			else if (strstr(new_cmd[0], "$")){
				char* var;

				for (int i = 0; i < getLength(new); i++){
					var = strtok(new_cmd[i], "$");
					ev_index = *var - 'a';
					printf("%s\n", env_vars[ev_index]);
				}
			}

			waitpid(pid, &retval,0);
			fprintf(stderr, "+ completed '%s'[%d]\n", cmd, WEXITSTATUS(retval));
		}
		else{
			perror("Error:");
			exit(1);
		}
	}
    return EXIT_SUCCESS;
}
