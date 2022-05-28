#include "mcc.h"
#include "symtbl.h"
#include "type.h"
#include "scope.h"


Symbol*     str_head;
Symbol*     str_tail;

static int st_unique_no(){
    static int no = 0;
    return no++;
}

void st_init(){
    sc_init_scope();
}

Symbol* st_make_symbol(Token* tok, Type* ty){
    Symbol* sym = calloc(1, sizeof(Symbol));

    sym->name = calloc(1, sizeof(char) * (tok->len + 1));
    strncpy(sym->name, tok->str, tok->len);
    sym->len = tok->len;
    sym->type = ty;

    return sym;
}

void st_declare(Symbol* sym){

    Type* ty = sym->type;
    // add stack size.
    if(!sc_is_grobal()){
        sc_add_funcstack(ty->size);
        sym->offset = sc_get_funcstack();
    } else {
        sym->is_grobalvar = true;
    }

    // chain symbol to current scope.
    sc_add_symbol(sym);
}

Symbol* st_find_symbol(Token* tok){

    for(Scope* scp = sc_get_cur_scope(); scp; scp = scp->parent){
        for(Symbol* sym = scp->symbol; sym; sym = sym->next){
            if(memcmp(sym->name, tok->str, tok->len) == 0
                && (sym->len == tok->len)){
                    return sym;
            }
        }
    }

    return NULL;
}

int st_get_stacksize(){
    return sc_get_funcstack();
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