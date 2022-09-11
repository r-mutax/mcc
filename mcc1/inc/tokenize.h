#ifndef TOKENIZE_INC_H
#define TOKENIZE_INC_H
#include "mcc1.h"

// function definition ----------------------------
Token*  tk_tokenize_file(char* path);
Token*  tk_tokenize(char* p);
bool    tk_iseof();
void    tk_expect(char* p);
int     tk_expect_num();
bool    tk_consume(char* p);
Token*  tk_consume_ident();
Token*  tk_expect_ident();
Type*   tk_consume_type();
Type*   tk_expect_type();
Type*   tk_consume_user_type();
char*   tk_getline();
bool    tk_consume_keyword(char* keyword);
void    tk_expect_keyword(char* keyword);
bool    tk_istype();
Token*  tk_get_token();
void    tk_set_token(Token* tok);
Token*  tk_consume_string();
bool    tk_is_label();
#endif
