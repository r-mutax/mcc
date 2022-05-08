#ifndef SYMTBL_INC_H
#define SYMTBL_INC_H
#include "tokenize.h"
#include "type.h"

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
    Type*       type;
};

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

#endif