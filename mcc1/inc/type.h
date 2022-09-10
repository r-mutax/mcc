#ifndef TYPE_INC_H
#define TYPE_INC_H
#include "mcc1.h"

void ty_init();
Type* ty_register_type(char* type_name, int type_size, TypeKind type_kind);
Type* ty_register_newtype(Symbol* new_name, Type* type);
Type* ty_get_type(char* type_name, int len);
Type* ty_pointer_to(Type* base_type);
Type* ty_array_of(Type* base_type, int array_len);
Symbol* ty_get_member(Type* ty, Token* tok);
void ty_add_type(Node* node);

void ty_register_enum(Type* enum_type);
Type* ty_find_enum(char* enum_name, int len);

void ty_register_struct(Type* struct_type);
Type* ty_find_struct(char* struct_name, int len);

#endif
