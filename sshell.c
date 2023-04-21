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


//This data structure represents each command as a node of data that each command is comprised of
struct node {
    char* file; //File for output redirect (check parse function to see how its set
    char** command; //String array representing the command (for execvp)
    struct node* next; 
    int length; //length of string array
    int parse_error;
    bool redir;
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
    n -> parse_error = 0;
    n -> redir = false;
    
    return(n);
}

//Accessors and setters for our node data structure

void setRedir(struct node* n){
    n -> redir = true;
}

bool getRedir(struct node* n){
    return n -> redir;
}


void setParse(struct node* n){
    n -> parse_error = 1;
}

int getParse(struct node* n){
    return n -> parse_error;
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

//This data structure represents our linked list of node commands
//The reason we need this is because when you work with multiple piping, you have to store multiple commands not just one
//EX: ls | sort | grep sshell will be represented as 3 nodes, the execvp arrays that they will be represented as are ["ls"] ["sort"], and ["grep","sshell"]


struct list {
    int *children;
    struct node* head;
    struct node* tail;
    struct node* curr;
    int length;
    int parse;
};

//make empty list
struct list* createList(void){
    struct list *l = malloc(sizeof(struct list));
    l -> tail = NULL;
    l -> head = NULL;
    l -> curr = NULL;
    l -> length = 0;
    l -> children = malloc(CMD_ARR_LEN * sizeof(int));
    l -> parse = 0;
	
    return(l);
}

//LinkedList Functions
void setListParse(struct list* l){
    l -> parse = 1;
}

int getListParse(struct list* l){
    return l -> parse;
}

void insert(struct list* l, struct node* n){
    if (getParse(n) == 1){
	setListParse(l);
    }

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

int* getExit(struct list* l){
    return l -> children;
}
    
//Parse takes in a string command, and then splits it up delimiting by spaces and stores it inside the node as the command array

int parse(struct node* n, char* string, char** env_vars){
    char temp[CMDLINE_MAX];
    strcpy(temp, string);

    char* ptr;
    //char* prev;
    char ptr2[CMD_LEN];
    int c = 0;
    ptr = strtok(temp, " ");
    while (ptr != NULL){
	//If your at the argument limit, you essentially error out and return
	if (c >= CMD_ARR_LEN){
	    fprintf(stderr,"Error: too many process arguments\n");
	    setParse(n);
	    return 1;
	}
	if (strcmp(ptr,">&") == 0){


	    ptr = strtok(NULL," ");
	    if (ptr != NULL){
		int temp3;
		temp3 = open(ptr, O_WRONLY | O_CREAT, 0644);
		if (temp3 == -1){
		    fprintf(stderr,"Error: cannot open output file\n");
		    setParse(n);
		    return 1;
		}
		close(temp3);
		setRedir(n);
		strcpy(n -> file, ptr);
		ptr = strtok(NULL, " ");
		continue;
	    }
	    else{
		n -> file = NULL;
		fprintf(stderr,"Error: no output file\n");
		setParse(n);
		return 1;
	    }
	}

	//This is for environment variables, if you see a valid var, the actual command gets altered to the value of the env var
    	if (ptr[0] == '$'){
	    strcpy(ptr2,ptr+1);
	    
	    if (ptr2 != NULL){
		if (strlen(ptr2) == 1 && islower(*ptr2)){
		    strcpy(n -> command[c],env_vars[*ptr2 - 'a']);
		    ptr = strtok(NULL, " ");
		    c += 1;
		    continue;
		}
		else{
		    fprintf(stderr,"Error: invalid variable name\n");
		    setParse(n);
		    return 1;   
		}
	    } 
	}

	//Output redirection
	if (strcmp(ptr, ">") == 0){
	    ptr = strtok(NULL, " ");
	    if (ptr != NULL){
		int temp;
		temp = open(ptr, O_WRONLY | O_CREAT, 0644);
		if (temp == -1){
		    fprintf(stderr,"Error: cannot open output file\n");
		    setParse(n);
		    return 1;
		}
		close(temp);
		strcpy(n -> file, ptr);
		ptr = strtok(NULL, " ");
		continue;
	    }
	    else{
		n -> file = NULL;
		fprintf(stderr,"Error: no output file\n");
		setParse(n);
		return 1;
	    }
	}
	strcpy(n -> command[c],ptr);
	ptr = strtok(NULL," ");
	c += 1;
    }
    n -> length = c;
    n -> command[c] = '\0';
    return 0;
}

//Pipeline forks our list of commands into child processes which will execvp the commands and then die
//To pipe or pass data between the commands we utilize the pipe syscall in a for loop structure where 
//at the end of each iteration the file descriptor that reads from the current pipe is passed to the next 
//iteration to be used as input to second command
void pipeline(struct list* l){
    int length = getLen(l);
    int status;
    int fd[2];
    int prev;
    int output;
    char file[FILE_LEN];
    char** command;
    prev = STDIN_FILENO; //This variable holds the fd to read from pipe of previous command
    front(l);

    for (int i = 0; i < length; i++){
	pipe(fd);
	command = getCommand(view(l));

	//exit if command is cd or pwd, because they are built-in and will be handled by parent in main
	if (strcmp(command[0], "cd") == 0){
	    return;
	}
	if (strcmp(command[0], "pwd") == 0){
	    return;
	}
	if (strcmp(command[0], "set") == 0){
	    return;
	}
	l -> children[i] = fork(); //We store our pids in an array and then reference this later on for exit codes (will see in main)
	if (l -> children[i] == 0){
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
		if (getRedir(view(l))){
		    dup2(output, STDERR_FILENO);
		    fflush(stderr);

		}
		close(output);
	    }	
	    execvp(command[0],command);
	    //If execvp doesnt kill child, we know an error happened, we exit(1) to send this as error exit code
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

    //We wait for all children using a loop, and then as each of them finish we update their exit status in children array using WEXITSTATUS
    for (int i =0; i <length;i++){
	waitpid(l -> children[i],&status,0);
	l -> children[i] = WEXITSTATUS(status); 
    }
}

int main(void){
	char cmd[CMDLINE_MAX]; 
	int* children;
	char* env_vars[26];
	int ev_index;

	for (int i = 0; i < 26; i++){
	    env_vars[i] = "";
	}

	while (1) {
	    char buf[BUF_MAX];
	    char *nl;
	    char** new_cmd;
	    
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
	    
	    //Parse buffer by pipes so that we get seperate commands, that will then be parsed by spaces into our execvp arrays
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
		parse(n, store_commands[i],env_vars);
		insert(a, n);
		if (getListParse(a) == 1){
		    break;
		}
	    }
	    if (getListParse(a) == 1){
		continue;
	    }
	    if (getLen(a) == 0){
		continue;
	    }
	    front(a);
	    new_cmd = getCommand(view(a));
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
	    }
	    else if (strcmp(new_cmd[0], "pwd") == 0){
		built = true;
		temp = 0;
		printf("%s\n", getcwd(buf, BUF_MAX));
	    }
		

	    else if (strcmp(new_cmd[0], "set") == 0){
	        built = true;

	    	if (getLength(view(a)) == 1){
		    fprintf(stderr,"Error: invalid variable name\n");
		    temp = 1;
		}
		else if (islower(*new_cmd[1])){
		    if (strlen(new_cmd[1]) == 1){
			ev_index = *new_cmd[1] - 'a';
			env_vars[ev_index] = new_cmd[2];
			temp = 0;
		    }
		    else{
			fprintf(stderr,"Error: invalid variable name\n");
			temp = 1;
		    }
		}
		else{
		    fprintf(stderr,"Error: invalid variable name\n");
		    temp = 1;
		}
	    }	
	   

	    pipeline(a);
	    children = getExit(a);

	    //We print out all of our WEXITSTATUSES here
	    //Done
	    if (built){
		fprintf(stderr, "+ completed '%s' [%d]\n",cmd, temp);
	    }
	    else{
		fprintf(stderr, "+ completed '%s' ", cmd);
		for (int i = 0 ; i < getLen(a); i++){
		    fprintf(stderr,"[%d] ",children[i]);				
		}
		fprintf(stderr,"\n");
		
	    }
	}
	return EXIT_SUCCESS;
}
