#include "mcc.h"
#include "tokenize.h"
#include "errormsg.h"

Token* token;

// local function forward definition. ------------
Token* new_token(TokenKind kind, Token* cur, char* str);

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

        if(strchr("+-*/()", *p)){
            cur = new_token(TK_OPERAND, cur, p++);
            continue;
        }

        if(isdigit(*p)){
            cur = new_token(TK_NUM, cur, p);
            cur->val = strtol(p, &p, 10);
            continue;
        }
    }

    new_token(TK_EOF, cur, p);
    token = head.next;
    return head.next;
}

bool tk_iseof(){
    return (token->kind == TK_EOF);
}

void tk_expect(char p){
    if(token->kind != TK_OPERAND
        || *(token->str) != p)
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

bool tk_consume(char op) {
    if (token->kind != TK_OPERAND || token->str[0] != op)
        return false;
    token = token->next;
    return true;
}

// local function ---------------------------------
Token* new_token(TokenKind kind, Token* cur, char* str)
{
    Token* tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    cur->next = tok;
    return tok;
}

