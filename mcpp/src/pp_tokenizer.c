#include "mcpp.h"

PP_Token* token = NULL;
SrcFile* cur_file = NULL;
unsigned long row = 0;

static PP_Token* ptk_tokenize(char* path);

PP_Token* ptk_tokenize_file(char* path){

    SrcFile* srcfile = calloc(1, sizeof(SrcFile));
    srcfile->input_data = read_file(path);
    srcfile->path = strdup(path);

    free(cur_file);
    cur_file = srcfile;

    PP_Token* tok = ptk_tokenize(cur_file->input_data);

    return NULL;
}

static PP_Token* ptk_tokenize(char* p){
    PP_Token head;
    head.next = NULL;
    PP_Token* cur = &head;
    row = 1;

    while(*p){      

        error("find cannnot tokenize words.\n");
    }
    return head.next;
}