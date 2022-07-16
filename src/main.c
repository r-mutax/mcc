#include "mcc.h"
#include "tokenize.h"
#include "node.h"
#include "codegen.h"
#include "semantics.h"
#include "preprocessor.h"
#include "file.h"
#include "errormsg.h"

char* src_file = NULL;
FILE* output_file = NULL;

void init_include_directory(int argc, char** argv){

    char cwd[256] = { 0 };
    char* pos = strrchr(argv[0], '/');
    strncpy(cwd, argv[0], pos - argv[0] + 1); 
    char* p = cwd;
    sprintf(cwd, "%s/stdlib/inc/", p);
    //register_include_directory(cwd);

    memset(cwd, 0, sizeof(cwd));
    pos = strrchr(src_file, '/');
    strncpy(cwd, src_file, pos - src_file + 1); 
    register_include_directory(cwd);
}

int read_option(char argc, char** argv){

    MCC_OPTION opt = C_SRC_FILE;

    for(char i = 0; i < argc; i++){
        if(argv[i][0] == '-'){
            switch(argv[i][1]){
                case 'i':
                case 'I':
                    opt = INCLUDE_PATH;
                    break;
                case 'c':
                case 'C':
                    opt = C_SRC_FILE;
                    break;
                case 'o':
                case 'O':
                    opt = OUTPUT_FILE;
                    break;
            }
            continue;
        }

        switch(opt){
            case NO_OPTION:
                break;
            case INCLUDE_PATH:
                register_include_directory(argv[i]);
                break;
            case C_SRC_FILE:
                src_file = argv[i];
                break;
            case OUTPUT_FILE:
                output_file = fopen(argv[i], "w");
                break;
            default:
                break;
        }
    }
}

int main(int argc, char **argv){

    if(argc < 2){
        fprintf(stderr, "mcc: error: Invalid Argument num.\n");
    }

    init_preprocess();
    
    output_file = stdout;
    read_option(argc, argv);
    init_include_directory(argc, argv);

    Token* tok = tk_tokenize_file(src_file);
    tk_set_token(tok);

    Program* program = parser();
    semantics_check(program);
    gen_program(program);

    return 0;
}