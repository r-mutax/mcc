#include "mcc.h"
#include "tokenize.h"

Token* token;

// local function forward definition. ------------
Token* new_token(TokenKind kind, Token* cur, char* str);
void error(char *fmt, ...);
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

        if(strchr("+-", *p)){
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
        error("expect %c, but get %c\n", p, token->str[0]);
    }
    token = token->next;
}

int tk_expect_num(){
    if(token->kind != TK_NUM){
        error("expect number.\n");
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

void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}