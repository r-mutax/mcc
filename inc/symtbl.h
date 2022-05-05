#ifndef SYMTBL_INC_H
#define SYMTBL_INC_H
#include "tokenize.h"

typedef struct Symbol Symbol;
typedef struct Scope Scope;

typedef enum {
    SC_GROBAL = 0,
    SC_FUNCTION,
    SC_BLOCK
} ScopeKind;


struct Symbol{
    Symbol*     next;
    char*       name;
    int         len;
    int         offset;
};

struct Scope{
    ScopeKind   kind;
    Symbol*     symbol;
    int         stacksize;
    Scope*      child;
    Scope*      parent;
};

void st_init();
void st_declare(Token* tok);
Symbol* st_find_symbol(Token* tok);
void st_start_scope();
void st_end_scope();

#endif