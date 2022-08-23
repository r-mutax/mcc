#include "mcpp.h"

void print_token(PP_Token* tok){
    while(tok){
        if(tok->kind == PTK_STRING_CONST){
            printf("\"");
        }
        printf("%s", tok->str);
        if(tok->kind == PTK_STRING_CONST){
            printf("\"");
        }
        tok = tok->next;
    }
}