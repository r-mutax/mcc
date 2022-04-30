#include <stdio.h>
#include <stdlib.h>
#include "mcc.h"

int main(int argc, char **argv){

    if(argc != 2){
        fprintf(stderr, "mcc: error: Invalid Argument num.");
    }

    printf(".intel_syntax noprefix\n");
    printf(".globl main\n");
    printf("main:\n");
    printf("  mov rax, %d\n", atoi(argv[1]));
    printf("  ret\n");

    return 0;
}