#include "mcc.h"

static char* user_input;
char* filename;

void error_init(char* input, char* fname){
    user_input = input;
    filename = fname;
}

void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

void error_at(char* loc, char *msg){

    char* line = loc;
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