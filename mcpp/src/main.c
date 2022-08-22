#include "mcpp.h"

#define STDLIB_PATH "/usr/include/"
#define ADDITIVE_STDLIB_PATH "/usr/lib/gcc/x86_64-linux-gnu/9/include/"
#define CSTDLIB_INC_PATH "/usr/include/x86_64-linux-gnu/"

static void read_arguments(int argc, char** argv);

char* src_path = NULL;
FILE* output_file = NULL;

int main(int argc, char** argv){

    if(argc < 2){
        fprintf(stderr, "mcpp: error: Invalid Argument num.\n");
    }

    // init output_file with stdout.
    output_file = stdout;

    read_arguments(argc, argv);

    // Add Srcfile directory to include path
    char cdir[256] = { 0 };
    get_file_directory(src_path, cdir);
    add_include_path(cdir);

    // Add Stdlib directory to std-include path
    add_std_include_path(STDLIB_PATH);
    add_std_include_path(ADDITIVE_STDLIB_PATH);
    add_std_include_path(CSTDLIB_INC_PATH);

    // init-preprocess
    init_preprocess();

    PP_Token* tok = ptk_tokenize_file(src_path);
    tok = preprocess(tok);

    output_preprocessed_file(tok, output_file);

    return 0;
}

static void read_arguments(int argc, char** argv){

    MCPP_OPTION opt = SRC_FILE_PATH;

    for(int i = 0; i < argc; i++){
        if(argv[i][0] == '-'){
            switch(argv[i][1]){
                case 'i':
                case 'I':
                    opt = INCLUDE_PATH;
                    break;
                case 'c':
                case 'C':
                    opt = SRC_FILE_PATH;
                    break;
                case 'o':
                case 'O':
                    opt = OUTPUT_FILE;
                    break;
            }
            continue;
        }

        switch(opt){
            case INCLUDE_PATH:
                add_include_path(argv[i]);
                break;
            case SRC_FILE_PATH:
                src_path = argv[i];
                break;
            case OUTPUT_FILE:
                output_file = fopen(argv[i], "w");
                break;
        }
    }
}