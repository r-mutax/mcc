#ifndef SYMTBL_INC_H
#define SYMTBL_INC_H
#include "mcc.h"

typedef struct Scope Scope;

typedef enum {
    SC_GROBAL = 0,
    SC_FUNCTION,
    SC_BLOCK
} ScopeKind;

struct Scope{
    ScopeKind   kind;
    Symbol*     symbol;
    int         stacksize;
    Scope*      child;
    Scope*      parent;
};

void st_init();
Symbol* st_declare(Token* tok, Type* ty);
Symbol* st_find_symbol(Token* tok);
void st_start_scope();
void st_end_scope();
int st_get_stacksize();
Symbol* st_copy_symbol(Symbol* sym);
Symbol* st_register_literal(Token* tok);
Symbol* st_get_string_list();

#endif