#include "mcc.h"
#include "type.h"

Type* type_head;
Type* type_tail;

void ty_init(){
    ty_register_type("long", 8, TY_INTEGER);
}

Type* ty_register_type(char* type_name, int type_size, TypeKind type_kind){

    Type* ty = calloc(1, sizeof(Type));

    ty->name = type_name;
    ty->len = strlen(type_name);
    ty->size = type_size;
    ty->kind = type_kind;

    if(type_head == NULL){
        type_head = ty;
        type_tail = ty;
    } else {
        type_tail->next = ty;
        type_tail = ty;
    }
}

Type* ty_get_type(char* type_name, int len){
    for(Type* cur = type_head; cur; cur = cur->next){
        if(cur->len == len && memcmp(cur->name, type_name, len) == 0){
            return cur;
        }
    }
    return NULL;
}