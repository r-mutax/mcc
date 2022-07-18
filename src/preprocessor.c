#include "preprocessor.h"
#include "tokenize.h"
#include "file.h"
#include "errormsg.h"
#include "utility.h"


static Token* del_newline(Token* tok);
static bool equal_token(char* directive, Token* tok);
static Token* read_include(char* path);
static Token* read_stdlib_include(char* path);
static Token* get_end_token(Token* inc);
static bool equal_token(char* directive, Token* tok);
static void add_macro(Token* def, Token* val);
static Token* make_copy_token(Token* src);
static Macro* find_macro(Token* tok);
static Token* analyze_ifdef(Token* tok, Token** tail, bool is_ifdef);
static Token* find_newline(Token* tok);
static char* get_header_path(Token* tok);

static Macro* macro;

Token* preprocess(Token* tok){

    Token head;
    Token* cur = &head;
    head.next = tok;

    do{
        Token* target = cur->next;

        if(target->kind == TK_PREPROCESS){
            // preprocessor directive
            if(equal_token("#include", target)){

                char* inc_path = get_header_path(target->next);

                Token* inc = NULL;
                if(!equal_token("<", target->next)){
                    inc = read_include(inc_path);
                }

                if(inc == NULL){
                    inc = read_stdlib_include(inc_path);
                    if(inc == NULL){
                        error_at(target->next, "Can not find header file.\n");
                    }
                }

                cur->next = inc;
                Token* tail = get_end_token(inc);
                Token* new_line = find_newline(target);
                tail->next = new_line->next;
                cur = tail;
                continue;
            } else if(equal_token("#define", target)){
                Token* def = target->next;
                Token* val = def->next;
                add_macro(def, val);

                cur->next = val->next;
                continue;
            } else if(equal_token("#ifdef", target)){
                Token* endif;
                Token* ifdef = analyze_ifdef(target->next, &endif, true);

                cur->next = ifdef;
                cur = ifdef;
                continue;
            } else if(equal_token("#ifndef", target)){
                Token* endif;
                Token* ifdef = analyze_ifdef(target->next, &endif, false);

                cur->next = ifdef;
                cur = ifdef;
                continue;
            }
        } else {
            Macro* mac = find_macro(target);
            if(mac){
                Token* new_token = make_copy_token(mac->val);

                new_token->next = target->next;
                cur->next = new_token;
            }
        }

        cur = cur->next;

    } while(cur->kind != TK_EOF);

    tok = head.next;

    tok = del_newline(tok);

    return tok;
}

void init_preprocess(){

    Token tok,tok2;
    tok.kind = TK_IDENT;
    tok.str = "MCC_COMPILER";
    tok.len = strlen(tok.str);

    tok2.kind = TK_NEWLINE;
    add_macro(&tok, &tok2);
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

static Token* read_include(char* path){

    Token* inc = NULL;
    char* inc_path = get_include_path(path);

    if(inc_path){
        inc = tk_tokenize_file(inc_path);
    }

    return inc;
}

static Token* read_stdlib_include(char* path){

    char* inc_path = calloc(1, strlen(path) + strlen(STDLIB_PATH));
    strcat(inc_path, STDLIB_PATH);
    strcat(inc_path, path);
    Token* inc = tk_tokenize_file(inc_path);

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

static void add_macro(Token* def, Token* val){
    Macro* mac = calloc(1, sizeof(Macro));

    mac->def = make_copy_token(def);
    mac->val = make_copy_token(val);

    mac->next = macro;
    macro = mac;
}

static Token* make_copy_token(Token* src){
    Token* new_tok = calloc(1, sizeof(Token));
    memcpy(new_tok, src, sizeof(Token));
    new_tok->next = NULL;

    return new_tok;
}

static Macro* find_macro(Token* tok){

    for(Macro* cur = macro; cur; cur = cur->next){
        if(equal_token(tok->str, cur->def)){
            return cur;
        }
    }

    return NULL;
}

static Token* analyze_ifdef(Token* tok, Token** tail, bool is_ifdef){

    bool is_defined = is_ifdef == (find_macro(tok) != NULL);
    Token* t_head = tok->next;
    Token* t_tail = NULL;
    Token* f_head = NULL;
    Token* f_tail = NULL;
    bool tf_path = true;

    Token head;
    Token* cur = &head;
    head.next = tok->next;

    while(cur){
        Token* target = cur->next;

        if(equal_token("#endif", target)){
            cur->next = target->next;
            if(tf_path){
                t_tail = cur;
            } else {
                f_tail = cur;
                t_tail->next = cur->next;
            }
            break; 
        } else if(equal_token("#else", target)){
            tf_path = false;
            t_tail = cur;
            f_head = target->next;
        } else if(equal_token("#ifdef", target)){
            Token* ifdef_tail;
            Token* ifdef_head = analyze_ifdef(target->next, &ifdef_tail, true);

            cur->next = ifdef_head;
            cur = ifdef_tail;
        } else if(equal_token("#ifndef", target)){
            Token* ifdef_tail;
            Token* ifdef_head = analyze_ifdef(target->next, &ifdef_tail, false);

            cur->next = ifdef_head;
            cur = ifdef_tail;
        }
        cur = cur->next;
    }

    if(is_defined){
        if(t_tail)
            *tail = t_tail;

        else {
            *tail = cur->next;
            t_head = *tail;
        }
        return t_head;
    } else {
        if(f_tail != NULL){
            *tail = f_tail;
            return f_head;
        } else {
            *tail = t_tail;
            return t_tail;
        }
    }
}

static Token* find_newline(Token* tok){
    Token* cur = tok;

    while(cur->kind != TK_NEWLINE){
        cur = cur->next;
        if(cur->next->kind == TK_EOF){
            error_at(cur, "Reach eof before find new-line token.\n");
        }
    }
    return cur;
}

static char* get_header_path(Token* tok){

    if(tok->kind == TK_STRING_CONST){
        return strndup(tok->str, tok->len);
    }

    if(tok->kind != TK_OPERAND || !equal_token("<", tok)){
        error_at(tok, "Expect include file path.\n");
    }

    int len = 0;
    Token* cur = tok->next;
    while(!equal_token(">", cur)){
        len += cur->len;
        cur = cur->next;
    }

    char* inc_path = calloc(1, len);
    cur = tok->next;
    while(!equal_token(">", cur)){
        strcat(inc_path, cur->str);
        cur = cur->next;
    }

    return inc_path;
}