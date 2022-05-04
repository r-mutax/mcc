#include "mcc.h"
#include "symtbl.h"

Scope       func_scope;

void st_declare(Token* tok){

    Symbol* sym = calloc(1, sizeof(Symbol));

    sym->name = tok->str;
    sym->len = tok->len;

    // add stack size.
    func_scope.stacksize += 8;

    sym->offset = func_scope.stacksize;

    // chain symbol to current scope.
    sym->next = func_scope.symbol;
    func_scope.symbol = sym;
}

Symbol* st_find_symbol(Token* tok){

    for(Symbol* sym = func_scope.symbol; sym; sym = sym->next){
        if(sym->len == tok->len 
            && memcmp(sym->name, tok->str, sym->len) == 0){
            return sym;
        }
    }
    return NULL;
}