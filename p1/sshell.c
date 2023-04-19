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
    char* file; //File for output redirect (check parse function to see how its set
    char** command; //String array representing the command (for execvp)
    struct node* next; 
    int length; 			//length of string array
};

//reworking data structures update***
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

struct list* createList(void){
    struct list *a = malloc(sizeof(struct list));
    a -> tail = NULL;
    a -> head = NULL;
    a -> curr = NULL;
    a -> length = 0;
    return(a);
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

void parse(struct node* a, char* string){
    char temp[CMDLINE_MAX];
    a -> command = (char**) malloc(CMD_ARR_LEN * sizeof(char *));

    for (int i = 0; i < 16; i++){
		a -> command[i] = (char*) malloc(CMD_LEN);
    }

    strcpy(temp,string);
    char* ptr;
    int c = 0;
    ptr = strtok(temp, " ");
    
	while (ptr != NULL){
		if (strcmp(ptr,">") == 0){
			ptr = strtok(NULL, " ");

			if (ptr != NULL){
				strcpy(a -> file, ptr);
				ptr = strtok(NULL," ");
				continue;
			}
			else{
				a -> file = NULL;
				break;
			}
		}

		strcpy(a -> command[c],ptr);
		ptr = strtok(NULL," ");
		c += 1;
    }
    a -> length = c;
    a -> command[c] = '\0';
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
    waitpid(p1, NULL, 0);
}

//Data structures are kinda complicated-ish but heres an example below of how to use them
int main(void){
	char cmd[CMDLINE_MAX];

	while (1) {
		char buf[BUF_MAX];
		char *nl;
		int retval;
		int fd;
		pid_t pid;
		struct node* new = createNode();

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
			break;
		}

		//parse the command line string into the node command object
		parse(new, cmd);
/* ======================== NILESH EDITS START ============================
		char copy_temp[CMDLINE_MAX];
		char** store_commands = (char**) malloc(16 * sizeof(char*));
		char* ptr;
		int c = 0;
		
		for (int i = 0; i < 16; i++){
		    store_commands[i] = (char*) malloc(32);
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

		pipeline(a);
// ======================== NILESH EDITS END ============================*/

		//declare vars for the commands and lengths and add a NULL to the end
		//of the array for the cmd and args so it's compatible with execvp
		char** new_cmd = getCommand(new);
		int new_len = getLength(new);
		new_cmd[new_len] = NULL;

		//fork to start the shell process executions
		pid = fork();

		//CHILD PROCESS
		if (pid == 0){
			
			//exit out of child process if commands cannot be executed with execvp
			if (strcmp(new_cmd[0], "cd") == 0){
				exit(0);
			}

			if (strcmp(new_cmd[0], "pwd") == 0){
				exit(0);
			}

			//point the stdout file descriptor to the redirection file 
			if (new -> file != NULL){
				fd = open(new -> file ,O_WRONLY | O_CREAT, 0644);
				dup2(fd, STDOUT_FILENO);
				close(fd);
			}

			execvp(new_cmd[0], new_cmd);
			perror("execvp");
			exit(1);
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
