#include "mcc.h"
#include "symtbl.h"
#include "type.h"

Scope       grobal_scope;
Scope*      cur_scope;
Scope*      func_scope;
int         scope_level = 0;

Symbol*     str_head;
Symbol*     str_tail;

static int st_unique_no(){
    static int no = 0;
    return no++;
}

void st_init(){
    cur_scope = &grobal_scope;
}

Symbol* st_declare(Token* tok, Type* ty){

    Symbol* sym = calloc(1, sizeof(Symbol));

    sym->name = calloc(1, sizeof(char) * (tok->len + 1));
    strncpy(sym->name, tok->str, tok->len);
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
            if(memcmp(sym->name, tok->str, tok->len) == 0
                && (sym->len == tok->len)){
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

Symbol* st_copy_symbol(Symbol* sym){
    Symbol* cpysym = calloc(1, sizeof(Symbol));
    memcpy(cpysym, sym, sizeof(Symbol));
    cpysym->next = NULL;
    return cpysym;
}

Symbol* st_register_literal(Token* tok){
    Symbol* sym = calloc(1, sizeof(Symbol));

    sym->str = calloc(1, sizeof(char) * (tok->len + 1));
    strncpy(sym->str, tok->str, tok->len);

    // string literal is including the null teminator.
    // so that length is string length + 1.
    sym->type = ty_get_type("char", 4);
    sym->type = ty_array_of(sym->type, tok->len + 1);

    sym->name = calloc(1, sizeof(char) * 20);
    sprintf(sym->name, ".LSTR%d", st_unique_no());

    sym->is_grobalvar = true;

    if(str_head){
        str_tail->next = sym;
        str_tail = sym;
    } else {
        str_head = str_tail = sym;
    }

    return sym;
}

Symbol* st_get_string_list(){
    return str_head;
}