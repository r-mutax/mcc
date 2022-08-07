#include "mcpp.h"

void error(char *fmt, ...){
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

void error_at(PP_Token* tok, char *msg){

    char* start = tok->src->input_data;
    int line = 1;

    do{
        if(line == tok->row){
            break;
        }
        
        if(*start == '\n'){
            line++;
        }

        start++;
    } while(*start);

    int indent = fprintf(stderr, "%s:%ld: ", get_filename(tok->src), tok->row);
    char* end = strchr(start, '\n');
    char* err_str = strndup(start, end - start + 1);
    fprintf(stderr, "%s", err_str);

    int pos = tok->pos - start + indent;
    fprintf(stderr, "%*s", pos, "");
    fprintf(stderr, "^ %s", msg);
    fprintf(stderr, "\n");
    exit(1);
}
