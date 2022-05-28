#include "scope.h"

Scope       grobal_scope;
Scope*      cur_scope;
Scope*      func_scope;
int         scope_level = 0;

void sc_init_scope(){
    cur_scope = &grobal_scope;
}

bool sc_is_grobal(){
    return cur_scope->kind == SC_GROBAL;
}

void sc_start_scope(){
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

void sc_end_scope(){
    Scope* scp = cur_scope;
    cur_scope = cur_scope->parent;
    scope_level--;
    
    if(scope_level == 0){
        // When end of Function Scope, reset func_scope.
        func_scope = NULL; 
    }

    free(scp);
}

void sc_add_funcstack(int size){
    func_scope->stacksize += size;
}

int sc_get_funcstack(){
    return func_scope->stacksize;
}

void sc_add_symbol(Symbol* sym){
    sym->next = cur_scope->symbol;
    cur_scope->symbol = sym;
}

Scope* sc_get_cur_scope(){
    return cur_scope;
}

void sc_add_type(Type* type){
    
    if(!cur_scope->type){
        cur_scope->type = type;
    } else {
        type->next = cur_scope->type;
        cur_scope->type = type;
    }
}