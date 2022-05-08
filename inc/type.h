#ifndef TYPE_INC_H
#define TYPE_INC_H
#include "mcc.h"

void ty_init();
Type* ty_register_type(char* type_name, int type_size, TypeKind type_kind);
Type* ty_get_type(char* type_name, int len);
void ty_add_type(Node* node);
#endif
