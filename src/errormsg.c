#include "mcc.h"

char* filename;

void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

void error_at(Token* tok, char *msg){

    char* loc = tok->str;
    char* line = loc;
    char* user_input = tok->src->input_data;
    while(user_input < line && line[-1] != '\n')
        line--;

    char* end = loc;
    while(*end != '\n')
        end++;
    
    int line_num = 1;
    for(char* p = user_input; p != line; p++)
        if(*p == '\n')
            line_num++;
    
    int indent = fprintf(stderr, "%s:%d: ", filename, line_num);
    fprintf(stderr, "%.*s\n", (int)(end - line), line);

    int pos = loc - line + indent;
    fprintf(stderr, "%*s", pos, "");
    fprintf(stderr, "^ %s", msg);
    fprintf(stderr, "\n");
    exit(1);
}