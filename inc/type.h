#ifndef TYPE_INC_H
#define TYPE_INC_H

typedef struct Type Type;

typedef enum {
    TY_INTEGER = 0
} TypeKind;

struct Type {
    Type*       next;

    TypeKind    kind;

    char*       name;
    int         len;

    int     size;
};

void ty_init();
Type* ty_register_type(char* type_name, int type_size, TypeKind type_kind);
Type* ty_get_type(char* type_name, int len);
#endif
