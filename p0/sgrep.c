#include <stdio.h>
#include <string.h>

#define COLOR_CODE	"\e[0;31m" /* Red */
#define RESET_CODE	"\e[0;0m"

int main(int argc, char* argv[]){

    if (argc <=2){
        printf("Usage: ./sgrep  [-n] [-c] [-h] [-p PATTERN]... FILE...\n");
    }
    else if (strcmp(argv[1], "-h") == 0){
        printf("Usage: ./sgrep  [-n] [-c] [-h] [-p PATTERN]... FILE...\n");
    }

    return 0;
}