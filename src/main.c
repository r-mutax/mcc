#include "mcc.h"
#include "tokenize.h"
#include "node.h"
#include "codegen.h"
#include "semantics.h"
#include "preprocessor.h"
#include "file.h"
#include "errormsg.h"

int main(int argc, char **argv){

    if(argc != 2){
        fprintf(stderr, "mcc: error: Invalid Argument num.");
    }

    Token* tok = tk_tokenize_file(argv[1]);
    preprocess(tok);

    Program* program = parser();
    semantics_check(program);
    gen_program(program);

    return 0;
}