#include "mcc.h"
#include "tokenize.h"
#include "node.h"
#include "codegen.h"
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

    Node* exp = parser();
    gen(exp);

    printf("  pop rax\n");
    printf("  ret\n");
    return 0;
}