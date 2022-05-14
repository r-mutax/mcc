#include "mcc.h"
#include "tokenize.h"
#include "type.h"
#include "errormsg.h"

Token* token;

// local function forward definition. ------------
static Token* new_token(TokenKind kind, Token* cur, char* str, int len);
static bool startswith(char* lhs, char* rhs);
static bool is_ident1(char p);
static bool is_ident2(char p);
static bool is_keyword(char* lhs, char* rhs);
static bool check_keyword(char* keyword, char** p, Token** tok);
// -----------------------------------------------
Token* tk_tokenize(char* p){

    Token head;
    head.next = NULL;
    Token* cur = &head;

    while(*p){

        // skip space, new line carrige return and so on.
        if(isspace(*p)){
            p++;
            continue;
        }

        if(startswith(p, "==")
            || startswith(p, "!=")
            || startswith(p, "<=")
            || startswith(p, ">="))
        {
            cur = new_token(TK_OPERAND, cur, p, 2);
            p += 2;
            continue;
        }

        if(strchr("+-*/,()<>;={}&[]%", *p)){
            cur = new_token(TK_OPERAND, cur, p++, 1);
            continue;
        }

        if(check_keyword("return", &p, &cur)
            || check_keyword("if", &p, &cur)
            || check_keyword("else", &p, &cur)
            || check_keyword("for", &p, &cur)
            || check_keyword("while", &p, &cur)
            || check_keyword("sizeof", &p, &cur)
            || check_keyword("int", &p, &cur)
            || check_keyword("long", &p, &cur)){
            continue;
        }

        if(is_ident1(*p)){
            char* start = p;
            p++;
            while(is_ident2(*p)){
                p++;
            }

            cur = new_token(TK_IDENT, cur, start, p - start);
            continue;
        }

        if('a' <= *p && *p <= 'z'){
            cur = new_token(TK_IDENT, cur, p++, 1);
            continue;
        }

        if(isdigit(*p)){
            cur = new_token(TK_NUM, cur, p, 0);
            char* q = p;
            cur->val = strtol(p, &p, 10);
            cur->len = p - q;
            continue;
        }
    }

    new_token(TK_EOF, cur, p, 0);
    token = head.next;
    return head.next;
}

bool tk_iseof(){
    return (token->kind == TK_EOF);
}

void tk_expect(char* p){
    if(token->kind != TK_OPERAND
        || token->len != strlen(p)
        || memcmp(p, token->str, token->len))
    {
        error_at(token->str, "expect %c, but get %c\n", p, token->str[0]);
    }
    token = token->next;
}

int tk_expect_num(){
    if(token->kind != TK_NUM){
        error_at(token->str, "expect number.\n");
    }
    int ret = token->val;
    token = token->next;
    return ret;
}

bool tk_consume(char* op) {
    if (token->kind != TK_OPERAND
         || token->len != strlen(op)
         || memcmp(op, token->str, token->len))
        return false;
    token = token->next;
    return true;
}

Token* tk_consume_ident(){
    if(token->kind != TK_IDENT){
        return NULL;
    }

    Token* tok = token;
    token = token->next;
    return tok;
}

Token*  tk_expect_ident(){
    if(token->kind != TK_IDENT){
        error_at(token->str, "Expect Identifier, but get another token.\n");
    }

    Token* tok = token;
    token = token->next;
    return tok;
}

char* tk_getline(){
    return token->str;
}

bool tk_consume_keyword(char* keyword){
    if(token->kind != TK_KEYWORD
        || token->len != strlen(keyword)
        || memcmp(keyword, token->str, token->len)){
        return false;
    }
    token = token->next;
    return true;
}

bool tk_istype(){
    Type* ty = ty_get_type(token->str, token->len);
    return ty != NULL;
}

Type* tk_consume_type(){
    Type* ty = ty_get_type(token->str, token->len);
    
    if(ty != NULL)
        token = token->next;
    return ty;
}

Type* tk_expect_type(){
    Type* ty = ty_get_type(token->str, token->len);
    if(ty == NULL){
        error_at(token->str, "Expect type.\n");
    }
    
    token = token->next;
    
    return ty;
}

Token* tk_get_token(){
    return token;
}

void tk_set_token(Token* tok){
    token = tok;
}

// local function ---------------------------------
static Token* new_token(TokenKind kind, Token* cur, char* str, int len){
    Token* tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    tok->len = len;
    cur->next = tok;
    return tok;
}

static bool startswith(char* lhs, char* rhs){
    return memcmp(lhs, rhs, strlen(rhs)) == 0;
}

static bool is_ident1(char p){
    return isalpha(p) || p == '_';
}

static bool is_ident2(char p){
    return is_ident1(p) || isdigit(p);
}

static bool is_keyword(char* lhs, char* rhs){

    if(memcmp(lhs, rhs, strlen(rhs)) == 0 && !is_ident2(lhs[strlen(rhs)])){
        return true;
    }
    return false;
}

// Check next token is keyword.
// Then add TK_KEYWORD token, and return true;
static bool check_keyword(char* keyword, char** p, Token** tok){

    if (is_keyword(*p, keyword)){
        int len = strlen(keyword);
        *tok = new_token(TK_KEYWORD, *tok, *p, len);
        *p += len;
        return true;
    }

    return false;
}
