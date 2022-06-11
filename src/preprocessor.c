#include "preprocessor.h"

void preprocess(Token* tok){

    Token* bef = NULL;
    for(Token* cur = tok; cur->kind != TK_EOF; cur = cur->next){
        if(cur->kind == TK_NEWLINE){
            if(bef){
                bef->next = cur->next;
            } else {
                tok = cur->next;
            }
        } else {
            bef = cur;
        }
    }
}