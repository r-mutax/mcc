#include "preprocessor.h"
#include "tokenize.h"
#include "file.h"
#include "errormsg.h"

static Token* del_newline(Token* tok);
static bool equal_token(char* directive, Token* tok);
static Token* read_include(Token* tok);
static Token* get_end_token(Token* inc);
static bool equal_token(char* directive, Token* tok);


Token* preprocess(Token* tok){

    Token head;
    Token* cur = &head;
    head.next = tok;

    do{
        Token* target = cur->next;

        if(target->kind == TK_PREPROCESS){
            // preprocessor directive

            if(equal_token("#include", target)){
                Token* path = target->next;
                Token* inc = read_include(target->next);

                cur->next = inc;
                Token* tail = get_end_token(inc);
                tail->next = path->next->next;
                cur = path->next;
            }
        }

        cur = cur->next;

    } while(cur->kind != TK_EOF);

    tok = head.next;

    tok = del_newline(tok);

    return tok;
}

// local function -------------------------------------------------
static Token* del_newline(Token* tok){

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

    return tok;
}

static bool equal_token(char* directive, Token* tok){

    if(tok->len == strlen(directive)
         && memcmp(tok->str, directive, tok->len) == 0){
        return true;
    }

    return false;
}

static Token* read_include(Token* tok){

    Token* path = tok;
    Token* new_line = path->next;

    if(path->kind != TK_STRING_CONST){
        error_at(path, "[error] expect string const.");
    }

    if(new_line->kind != TK_NEWLINE){
        error_at(new_line, "[error] continue some token exclude new-line after #include directive.");
    }

    // reaf header file.
    char* inc_path = calloc(1, path->len);
    strncpy(inc_path, path->str, path->len);
    inc_path = get_include_path(inc_path);
    Token* inc = tk_tokenize_file(inc_path);
    //inc = preprocess(inc);

    return inc;
}

static Token* get_end_token(Token* inc){
    
    Token* bef;

    while(inc->kind != TK_EOF){
        bef = inc;
        inc = inc->next;
    }

    return bef;
}
