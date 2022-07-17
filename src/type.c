#include "mcc.h"
#include "type.h"
#include "scope.h"
#include "errormsg.h"

void ty_init(){
    ty_register_type("long", 8, TY_INTEGER);
    ty_register_type("int", 4, TY_INTEGER);
    ty_register_type("char", 1, TY_INTEGER);
    ty_register_type("short", 2, TY_INTEGER);
    ty_register_type("_Bool", 1, TY_BOOL);
    ty_register_type("void", 0, TY_VOID);
}

Type* ty_register_type(char* type_name, int type_size, TypeKind type_kind){

    Type* ty = calloc(1, sizeof(Type));

    ty->name = type_name;
    ty->len = strlen(type_name);
    ty->size = type_size;
    ty->kind = type_kind;

    sc_add_type(ty);

    return ty;
}

Type* ty_register_newtype(Symbol* new_name, Type* type){

    Type* ty = ty_register_type(new_name->name, type->size, type->kind);

    ty->is_typedef = true;
    ty->base_type = type;
    return ty;
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

Type* ty_find_enum(char* enum_name, int len){
    
    for(Scope* sc = sc_get_cur_scope(); sc; sc = sc->parent){
        for(Type* cur = sc->type; cur; cur = cur->next){
            // match cur typename and cur is 'enum' type.
            if(cur->len == len
                && memcmp(cur->name, enum_name, len) == 0
                && cur->kind == TY_ENUM){
                return cur;
            }
        }
    }
}

void ty_register_enum(Type* enum_type){
    sc_add_type(enum_type);
}

void ty_register_struct(Type* struct_type){

    Type* ret = ty_find_struct(struct_type->name, struct_type->len);
    if(ret && ret->is_imcomplete){

        ret->is_imcomplete = false;
        ret->member = struct_type->member;
        ret->size = struct_type->size;
        return;
    }

    sc_add_struct_type(struct_type);
}

Type* ty_find_struct(char* struct_name, int len){
    
    for(Scope* sc = sc_get_cur_scope(); sc; sc = sc->parent){
        for(Type* cur = sc->str_type; cur; cur = cur->next){
            // match cur typename and cur is 'struct' type.
            if(cur->len == len
                && memcmp(cur->name, struct_name, len) == 0){
                return cur;
            }
        }
    }

}

Type* ty_get_type(char* type_name, int len){

    for(Scope* sc = sc_get_cur_scope(); sc; sc = sc->parent){
        for(Type* cur = sc->type; cur; cur = cur->next){
            if(cur->len == len && memcmp(cur->name, type_name, len) == 0){

                while(cur->base_type){
                    cur = cur->base_type;
                }

                return cur;
            }
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
            if((node->lhs->type->kind == TY_ENUM)
            && (node->lhs->type != node->rhs->type)){
                error("assign not enum value to enum.");
            }
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
            if(!node->lhs->type->pointer_to)
                error("invalid dereference pointer.\n");
            if(node->lhs->type->pointer_to->kind == TY_VOID)
                error("try dereference void pointer.\n");

            node->type = node->lhs->type->pointer_to;
            break;
        case ND_DECLARE:
            node->type = node->sym->type;
            break;
        case ND_BIT_LSHIFT:
        case ND_BIT_RSHIFT:
        case ND_BIT_AND:
        case ND_BIT_OR:
        case ND_BIT_XOR:
            node->type = node->lhs->type;
            break;
    }
}