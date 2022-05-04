#ifndef SYMTBL_INC_H
#define SYMTBL_INC_H
#include "tokenize.h"

typedef struct Symbol Symbol;
typedef struct Scope Scope;

struct Symbol{
    Symbol*     next;
    char*       name;
    int         len;
    int         offset;
};

struct Scope{
    Symbol*     symbol;
    int         stacksize;
};

void st_declare(Token* tok);
Symbol* st_find_symbol(Token* tok);

#endif