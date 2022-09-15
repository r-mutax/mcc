#ifndef TYPE_INC_H
#define TYPE_INC_H
#include "mcc1.h"

extern Type* ty_void;
extern Type* ty_Bool;

extern Type* ty_char;
extern Type* ty_short;
extern Type* ty_int;
extern Type* ty_long;

extern Type* ty_uchar;
extern Type* ty_ushort;
extern Type* ty_uint;
extern Type* ty_ulong;


void ty_init();
Type* ty_create_type(char* type_name, int type_size, TypeKind type_kind);
Type* ty_register_type(char* type_name, int type_size, TypeKind type_kind);
Type* ty_register_newtype(Symbol* new_name, Type* type);
Type* ty_get_type(char* type_name, int len);
Type* ty_find_user_type(char* type_name, int len);
Type* ty_pointer_to(Type* base_type);
Type* ty_array_of(Type* base_type, int array_len);
Symbol* ty_get_member(Type* ty, Token* tok);
void ty_add_type(Node* node);

void ty_register_enum(Type* enum_type);
Type* ty_find_enum(char* enum_name, int len);

void ty_register_struct(Type* struct_type);
Type* ty_find_struct(char* struct_name, int len);

#endif
