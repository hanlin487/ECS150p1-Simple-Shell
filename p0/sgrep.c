#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define COLOR_CODE	"\e[0;31m" /* Red */
#define RESET_CODE	"\e[0;0m"

struct node {
    char* val;
    struct node* next;
}; 

void printL(struct node* head) {
    struct node* ptr = head;
    
    while (ptr != NULL){
        printf("%s ", ptr->val);
        ptr = ptr->next;
    }
    printf("\n");
}

void freeL(struct node* head){
    struct node* ptr = head;

    while (head != NULL){
        ptr = head;
        head = head->next;
        free(ptr);
    }
}

void insertNode(char* arg, struct node** head){
    struct node* new = (struct node*) malloc(sizeof(struct node));

    new->val = arg;
    new->next = NULL;

    if (*head == NULL){
        *head = new;
    }
    else {
        struct node* ptr = *head;
        
        while(ptr->next != NULL){
            ptr = ptr->next;
        }
        ptr->next = new;
    }   
}

void parse(int c, int n, struct node* p, struct node* f){
    FILE *fptr;

    
}

int main(int argc, char* argv[]){

    struct node* cmd_list = NULL;
    struct node* temp;
    struct node* patterns = NULL;
    struct node* files = NULL;
    int colors = 0;
    int lines = 0;

    for (int i = 1; i < argc; i++){
        insertNode(argv[i], &cmd_list);
    }

    printf("command line list nodes: ");
    printL(cmd_list);
    temp = cmd_list;

    while (temp != NULL){
        if (strcmp(temp->val, "-h") == 0){
            printf("Usage: ./sgrep [-n] [-c] [-h] [-p PATTERN]... FILE...\n");
        }
        else if (strcmp(temp->val, "-c") == 0){
            colors = 1;
        }
        else if (strcmp(temp->val, "-n") == 0){
            lines = 1;
        }
        else if (strcmp(temp->val, "-p") == 0){
            temp = temp->next;
            insertNode(temp->val, &patterns);
        }
        else {
            insertNode(temp->val, &files);
        } 
        temp = temp->next;
    }
    
    printf("(-c: %d,-n: %d)\n", colors, lines);   
    
    printf("-p: "); 
    printL(patterns);
    
    printf("files: ");
    printL(files);

    parse(colors, lines, patterns, files);

    freeL(cmd_list);
    return 0;
}