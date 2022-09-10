#ifndef SYMTBL_INC_H
#define SYMTBL_INC_H
#include "mcc1.h"

void st_init();
Symbol* st_make_symbol(Token* tok, Type* ty);
void st_declare(Symbol* sym);
Symbol* st_find_symbol(Token* tok);
int st_get_stacksize();
Symbol* st_copy_symbol(Symbol* sym);
Symbol* st_register_literal(Token* tok);
Symbol* st_get_string_list();
void st_register_data(Symbol* sym);
Symbol* st_get_data_list();

#endif