#include "mcc.h"
#include "symtbl.h"

Scope       grobal_scope;
Scope*      cur_scope;
Scope*      func_scope;
int         scope_level = 0;


void st_init(){
    cur_scope = &grobal_scope;
}

Symbol* st_declare(Token* tok, Type* ty){

    Symbol* sym = calloc(1, sizeof(Symbol));

    sym->name = tok->str;
    sym->len = tok->len;
    sym->type = ty;

    // add stack size.
    if(cur_scope->kind != SC_GROBAL){
        if(ty->kind == TY_ARRAY){
            func_scope->stacksize += ty->size * ty->array_len;
        } else {
            func_scope->stacksize += ty->size;
        }
        sym->offset = func_scope->stacksize;
    } else {
        sym->is_grobalvar = true;
    }

    // chain symbol to current scope.
    sym->next = cur_scope->symbol;
    cur_scope->symbol = sym;

    return sym;
}

Symbol* st_find_symbol(Token* tok){

    for(Scope* scp = cur_scope; scp; scp = scp->parent){
        for(Symbol* sym = scp->symbol; sym; sym = sym->next){
            if(sym->len == tok->len
                && memcmp(sym->name, tok->str, sym->len) == 0){
                    return sym;
            }
        }
    }

    return NULL;
}

void st_start_scope(){
    Scope* scp = calloc(1, sizeof(Scope));

    scp->parent = cur_scope;
    cur_scope->child = scp;

    scope_level++;
    if(scope_level == 1){
        scp->kind = SC_FUNCTION;
        func_scope = scp;
    } else if(scope_level >= 2){
        scp->kind = SC_BLOCK;
    }

    cur_scope = scp;
}

void st_end_scope(){
    Scope* scp = cur_scope;
    cur_scope = cur_scope->parent;
    scope_level--;
    
    if(scope_level == 0){
        // When end of Function Scope, reset func_scope.
        func_scope = NULL; 
    }

    free(scp);
}

int st_get_stacksize(){
    return func_scope->stacksize;
}