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

    // prologue
    printf("  push rbp\n");
    printf("  mov rbp, rsp\n");
    printf("  sub rsp, 208\n");

    Program* program = parser();
    Node* cur = program->body;
    gen_compound_stmt(cur);

    gen_epilogue();
    return 0;
}