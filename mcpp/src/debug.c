#include "mcpp.h"

void print_token(PP_Token* tok){
    while(tok){
        printf("%s", tok->str);
        tok = tok->next;
    }
}