#include "mcpp.h"

char* src_path = NULL;

#define STDLIB_PATH "/usr/include/"
#define ADDITIVE_STDLIB_PATH "/usr/lib/gcc/x86_64-linux-gnu/9/include/"
#define CSTDLIB_INC_PATH "/usr/include/x86_64-linux-gnu/"


int main(int argc, char** argv){

    if(argc < 2){
        fprintf(stderr, "mcpp: error: Invalid Argument num.\n");
    }

    // Add Srcfile directory to include path
    char cdir[256] = { 0 };
    get_file_directory(argv[1], cdir);
    add_include_path(cdir);

    // Add Stdlib directory to std-include path
    add_std_include_path(STDLIB_PATH);
    add_std_include_path(ADDITIVE_STDLIB_PATH);
    add_std_include_path(CSTDLIB_INC_PATH);

    // init-preprocess
    init_preprocess();

    src_path = argv[1];
    PP_Token* tok = ptk_tokenize_file(src_path);
    tok = preprocess(tok);

    print_token(tok);

    return 0;
}