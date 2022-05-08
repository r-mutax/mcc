#ifndef TOKENIZE_INC_H
#define TOKENIZE_INC_H
#include "type.h"
// token data definition --------------------------
typedef enum {
    TK_OPERAND = 0,
    TK_NUM,
    TK_IDENT,
    TK_KEYWORD,
    TK_EOF
} TokenKind;

typedef struct Token Token;

struct Token { 
    TokenKind   kind;
    Token*      next;
    int         val;
    char*       str;
    int         len;
};

// function definition ----------------------------
Token*  tk_tokenize(char* p);
bool    tk_iseof();
void    tk_expect(char* p);
int     tk_expect_num();
bool    tk_consume(char* p);
Token*  tk_consume_ident();
Token*  tk_expect_ident();
Type*  tk_consume_type();
Type*  tk_expect_type();
char*   tk_getline();
bool    tk_consume_keyword(char* keyword);
bool tk_istype();
#endif
