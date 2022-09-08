#include "mcc1.h"

void sc_init_scope();
bool sc_is_grobal();
void sc_start_scope();
void sc_end_scope();

void sc_add_funcstack(int size);
int sc_get_funcstack();
void sc_add_symbol(Symbol* sym);
Scope* sc_get_cur_scope();

void sc_add_type(Type* type);
void sc_add_struct_type(Type* struct_type);