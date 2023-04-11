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
    FILE *fptr = NULL;
    char line[250];
    char* pos;
    //char* pre;
    int i, index;
    struct node* temp;

    //printf("%d %d\n", c, n);
    //printL(p);
    //printL(f);

    //while there's still files to read
    while (f != NULL){
        fptr = fopen(f->val, "r");
        if (fptr == NULL){
            printf("file unable to be opened\n");
            exit(1);
        }
        i = 1;

        //get lines from opened file
        while (fgets(line, 250, fptr) != NULL){
            //printf("%d: %s", i, line);
            /*if (strstr(line, p->val) != NULL){
                if (n == 1){
                    printf("%d: ", i);
                }
                printf("%s", line);
            }*/
            temp = p;

            //while there are still patterns to look for
            while (temp != NULL){
                //if the line contains the pattern
                if (strstr(line, temp->val) != NULL) {

                    //print line number 
                    if (n == 1) {
                        printf("%d: ", i);
                    }
                    //coloring option used
                    if (c == 1){
                        
                        pos = line;
                        index = 0;

                        //find the index at which the pattern word is at within the current line
                        while (pos != strstr(line, temp->val)){
                            index++;
                            pos++;
                        }
                        printf("%.*s", index, line);
                        printf(COLOR_CODE "%s" RESET_CODE, temp->val);
                        printf("%s", strstr(line, temp->val) + strlen(temp->val));
                    }
                    else {
                        printf("%s", line);
                    }

                }
                temp = temp->next;
            }
            i++;
        }
        f = f->next;
    }
    fclose(fptr);
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

    //printf("command line list nodes: ");
    //printL(cmd_list);
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
    
    /*printf("(-c: %d,-n: %d)\n", colors, lines);   
    
    printf("-p: "); 
    printL(patterns);
    
    printf("files: ");
    printL(files);*/
    if (patterns && files){
        parse(colors, lines, patterns, files);
    }

    freeL(cmd_list);
    freeL(files);
    freeL(patterns);
    return 0;
}