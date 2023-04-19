#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <fcntl.h>

#define CMDLINE_MAX 512

struct node {
    bool read_signal; //If the pipe symbol was before this command, set this to true
    bool write_signal; //If the pipe symbol was after this command, set this to true; if its sorrounded by pipes set both
    char* file; //File for output redirect (check parse function to see how its set
    char** command; //String array representing the command (for execvp)
    struct node* next; 
    int length; //length of string array
};

//reworking data structures update***
struct node* createNode(void){
    struct node *n = malloc(sizeof(struct node));
    n -> read_signal = false;
    n -> write_signal = false;
    n -> file = (char*) malloc(32);
    n -> command = (char**) malloc(16*sizeof(char*));
   
	for (int i = 0; i < 16; i++){
		n -> command[i] = (char*) malloc(32);
    }

    n -> file = (char*) malloc(32);
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

struct list {
    struct node* head;
    struct node* tail;
    struct node* curr;
};

struct list* createList(void){
    struct list *a = malloc(sizeof(struct list));
    a -> tail = NULL;
    a -> head = NULL;
    a -> curr = NULL;
    return(a);
}

void insert(struct list* l, struct node* n){
	if (l -> tail == NULL){
		l -> tail = n;
		l -> head = n;
		n -> next = NULL;
		return;
    }

    l -> tail -> next = n;
    n -> next = NULL;
    l -> tail = n;
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

//Because this parse is a node function I pretty much have to rewrite the strtok code in the main to delimit by "|"
void parse(struct node* a, char* string){
    //struct list *l = malloc(sizeof(struct list));
    char temp[CMDLINE_MAX];
    a -> command = (char**)malloc(16 * sizeof(char *));

    for (int i = 0; i < 16; i++){
		a -> command[i] = (char*)malloc(32);
    }

    strcpy(temp,string);
    char* ptr;
    int c = 0;
    ptr = strtok(temp, " ");
    
	while (ptr != NULL){
	//printf("%s\n",ptr);

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
}

void simple_pipe(struct list* l) {
	pid_t p1, p2;
	int fd[2];
	struct node* temp;

	front(l);
	temp = view(l);
	printf("TEMP CMD: %s\n", *getCommand(temp));

	pipe(fd);

	if(!(p1 = fork())){
		close(fd[0]);
		dup2(fd[1], STDOUT_FILENO);
        close(fd[1]);

	}

	if (!(p2 = fork())){
        close(fd[1]);
        dup2(fd[0], STDIN_FILENO);
        close(fd[0]);

    }

	close(fd[0]);
    close(fd[1]);
    waitpid(p1, NULL, 0);
    waitpid(p2, NULL, 0);
}

//Data structures are kinda complicated-ish but heres an example below of how to use them
int main(void){
	/*
	//In an actual terminal this would be inputted as ls -l | more, you could then just delimit the | to get 2 strngs
	char* string = "ls -l | more";
	char* string2 = "more";

	//Create an empty node
	struct node* a = createNode();

	//Call parse on that node, the string passed in will be seperated by spaces and turned into a string array of the node (command)
	//So here, "ls -l" would become an array ["ls","-l"] contained in node a
	parse(a,string);
	int len;
	char** t;

	//for (int i = 0; i < l ; i++){
	    //printf("%s\n",t[i]);
	//}

	//Same thing as node a, but for string2
	struct node* b = createNode();
	parse(b,string2);
	
	//Create a list and then insert both the nodes you just created
	struct list* l = createList();
	insert(l,a);
	insert(l,b);

	//Front to move curr to head
	front(l);

	//View to return curr node
	struct node* temp;
	temp = view(l);
	while (temp != NULL){
	    //get length and string array of node
	    len = getLength(temp);
	    t = getCommand(temp);

	    //print the string array
	    for (int i = 0; i < len; i++){
			printf("%s\n",t[i]);
	    }

	    //use right to move curr to the right in linked list
	    right(l);

	    //view again to update our while condition
	    temp = view(l);
	}

	//commented out code is my old main for old data structure I'll just tweak this slightly for new implementation
    */
	char cmd[CMDLINE_MAX];

	while (1) {
		char buf[100];
		char *nl;
		int retval;
		int fd;
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
			break;
		}
	
		/*struct list* a = createList();
		char** ds = getList(a);
		char* cmd_2 = ds[0];
		int len = getLength(a);
		ds[len] = '\0';
		char* file = getFile(a);
		int fd;
		//printf("%s\n",cmd_2);
		//execvp(cmd_2,ds);*/


		struct node* new = createNode();

		parse(new, cmd);
		
		char** new_cmd = getCommand(new);
		int new_len = getLength(new);
		new_cmd[new_len] = NULL;

		printf("NODE CMD: ");
		
		for (int i = 0; i < new_len; i++){
			printf("%s ", new_cmd[i]);
		}

		printf("\n");

		pid = fork();
		
		if (pid == 0){
			//child

			if (strcmp(new_cmd[0],"cd") == 0){
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

			if (strcmp(new_cmd[0],"cd") == 0){
				chdir(new_cmd[1]);
				printf("%s\n",getcwd(buf,100));
			}
			else if (strcmp(new_cmd[0], "pwd") == 0){
				printf("%s\n", getcwd(buf, 100));
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