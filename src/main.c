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


    Program* program = parser();
    gen_program(program);

    return 0;
}