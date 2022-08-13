#include "mcpp.h"

char* src_path = NULL;

int main(int argc, char** argv){

    if(argc < 2){
        fprintf(stderr, "mcpp: error: Invalid Argument num.\n");
    }

    char cdir[256] = { 0 };
    get_file_directory(argv[1], cdir);
    add_include_path(cdir);

    src_path = argv[1];
    PP_Token* tok = ptk_tokenize_file(src_path);
    tok = preprocess(tok);

    print_token(tok);

    return 0;
}