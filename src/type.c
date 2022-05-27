#include "mcc.h"
#include "type.h"

Type* type_head;
Type* type_tail;

void ty_init(){
    ty_register_type("long", 8, TY_INTEGER);
    ty_register_type("int", 4, TY_INTEGER);
    ty_register_type("char", 1, TY_INTEGER);
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

// In mcc Compiler type system,
// different length array is different type.
Type* ty_array_of(Type* base_type, int array_len){

    Type* ty = calloc(1, sizeof(Type));

    ty->kind = TY_ARRAY;
    ty->array_len = array_len;
    ty->size = base_type->size * ty->array_len;
    ty->pointer_to = base_type;

    return ty;
}

Type* ty_get_type(char* type_name, int len){
    for(Type* cur = type_head; cur; cur = cur->next){
        if(cur->len == len && memcmp(cur->name, type_name, len) == 0){
            return cur;
        }
    }
    return NULL;
}

Type* ty_pointer_to(Type* base_type){
    if(base_type->pointer_from){
        return base_type->pointer_from;
    }

    Type* type = calloc(1, sizeof(Type));

    type->kind = TY_POINTER;
    type->pointer_to = base_type;
    type->size = 8;
    base_type->pointer_from = type;

    return type;
}

Symbol* ty_get_member(Type* ty, Token* tok){
    for(Member* cur = ty->member; cur; cur = cur->next){
        Symbol* sym = cur->sym;
        if(sym->len == tok->len && memcmp(sym->name, tok->str, tok->len) == 0){
            return sym;
        }
    }
    return NULL;
}

void ty_add_type(Node* node){

    if(node == NULL || node->type){
        return;
    }

    ty_add_type(node->lhs);
    ty_add_type(node->rhs);
    ty_add_type(node->cond);
    ty_add_type(node->body);
    ty_add_type(node->init);
    ty_add_type(node->iter);
    ty_add_type(node->else_body);
    ty_add_type(node->next);
    ty_add_type(node->arg);

    switch(node->kind){
        case ND_ADD:
        case ND_SUB:
        case ND_MUL:
        case ND_DIV:
        case ND_MOD:
        case ND_ASSIGN:
            node->type = node->lhs->type;
            break;
        case ND_EQ:
        case ND_NE:
        case ND_LT:
        case ND_LE:
        case ND_NUM:
            node->type = ty_get_type("long", 4);
            break;
        case ND_ADDR:
            node->type = ty_pointer_to(node->lhs->type);
            break;
        case ND_DREF:
            if(node->lhs->type->pointer_to){
                node->type = node->lhs->type->pointer_to;
            } else {
                node->type = ty_get_type("long", 4);
            }
            break;
        case ND_DECLARE:
            node->type = node->sym->type;
            break;
    }
}