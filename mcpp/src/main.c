#include "mcpp.h"

char* src_path = NULL;

int main(int argc, char** argv){

    if(argc < 2){
        fprintf(stderr, "mcpp: error: Invalid Argument num.\n");
    }

    src_path = argv[1];
    PP_Token* tok = ptk_tokenize_file(src_path);

    return 0;
}