#include "mcc.h"
#include "tokenize.h"
#include "errormsg.h"

Token* token;

// local function forward definition. ------------
Token* new_token(TokenKind kind, Token* cur, char* str, int len);
bool startswith(char* lhs, char* rhs);

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

        if(strchr("+-*/()<>;=", *p)){
            cur = new_token(TK_OPERAND, cur, p++, 1);
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

char* tk_getline(){
    return token->str;
}

// local function ---------------------------------
Token* new_token(TokenKind kind, Token* cur, char* str, int len){
    Token* tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    tok->len = len;
    cur->next = tok;
    return tok;
}

bool startswith(char* lhs, char* rhs){
    return memcmp(lhs, rhs, strlen(rhs)) == 0;
}