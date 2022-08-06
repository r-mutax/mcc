#include "mcpp.h"

static Macro* macro;

static PP_KIND get_preprocess_kind(PP_Token* token);
static bool equal_token(const char* word, PP_Token* target);
static void add_macro(PP_Token* tok);

// preprocess exchange 
PP_Token* preprocess(PP_Token* tok){

    PP_Token head = { 0 };
    PP_Token* cur = &head;
    head.next = tok;

    do {
        PP_Token* target = cur->next;

        if(target->kind == PTK_HASH){
            
            switch(get_preprocess_kind(target)){
                case PP_INCLUDE:
                    break;
                case PP_IF:
                case PP_IFDEF:
                case PP_IFNDEF:
                    break;
                case PP_ERROR:
                    break;
                case PP_DEFINE:
                    break;
                case PP_UNDEF:
                    break;
                default:
                    error("unexpected preprocessor directive.\n");
                    break;
            }
        } else {
            // replace token
        }
    } while(cur->kind != PTK_EOF);

    return NULL;
}

static bool equal_token(const char* word, PP_Token* target){
    if(target->len == strlen(word)
        && memcmp(target->str, word, target->len) == 0){
        return true;
    }

    return false;
}

static PP_KIND get_preprocess_kind(PP_Token* token){

    PP_Token* target = token->next;
    if(target->kind == PTK_SPACE){
        target = target->next;
    }

    PP_KIND kind = PP_NONE;
    if(equal_token("include", token)){
        kind = PP_INCLUDE;
    } else if(equal_token("if", token)){
        kind = PP_IF;
    } else if(equal_token("ifdef", token)){
        kind = PP_IFDEF;
    } else if(equal_token("ifndef", token)){
        kind = PP_IFNDEF;
    } else if(equal_token("else", token)){
        kind = PP_ELSE;
    } else if(equal_token("endif", token)){
        kind = PP_ENDIF;
    } else if(equal_token("error", token)){
        kind = PP_ERROR;
    } else if(equal_token("define", token)){
        kind = PP_DEFINE;
    } else if(equal_token("defined", token)){
        kind = PP_DEFINED;
    } else if(equal_token("undef", token)){
        kind = PP_UNDEF;
    }

    return kind;
}