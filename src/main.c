#include "mcc.h"
#include "tokenize.h"
#include "errormsg.h"

int main(int argc, char **argv){

    if(argc != 2){
        fprintf(stderr, "mcc: error: Invalid Argument num.");
    }

    error_init(argv[1]);
    tk_tokenize(argv[1]);

    printf(".intel_syntax noprefix\n");
    printf(".globl main\n");
    printf("main:\n");
    printf("  mov rax, %d\n", tk_expect_num());

    while(!tk_iseof()){
        if(tk_consume('+')){
            printf("  add rax, %d\n", tk_expect_num());
            continue;
        }

        tk_expect('-');
        printf("  sub rax, %d\n", tk_expect_num());
    }
    printf("  ret\n");
    return 0;
}