#ifndef NODE_INC_H
#define NODE_INC_H
#include "symtbl.h"

// parser data definition ----------------------------------
typedef enum {
    ND_ADD = 0,
    ND_SUB,
    ND_MUL,
    ND_DIV,
    ND_MOD,
    ND_DREF,
    ND_ADDR,
    ND_NUM,
    ND_EQ,
    ND_NE,
    ND_LT,
    ND_LE,
    ND_ASSIGN,
    ND_LVAR,
    ND_RETURN,
    ND_IF,
    ND_WHILE,
    ND_CMPDSTMT,
    ND_FOR,
    ND_CALL,
    ND_DECLARE
} NodeKind;

typedef struct Node Node;
typedef struct Program Program;
typedef struct Function Function;

struct Node {
    NodeKind    kind;
    Node*       lhs;
    Node*       rhs;
    int         val;
    int         offset;

    Node*       next;

    Node*       cond;
    Node*       body;
    Node*       init;
    Node*       iter;
    Node*       else_body;

    char*       func_name;
    int         len;
    Node*       arg;

    char*       line;
};

struct Function {
    char*       name;
    int         len;
    int         stack_size;
    Type*       ret_type;

    Node*       body;
    Node*       param;
    int         paramnum;
    Function*   next;
};

struct Program {
    Function*   func_list;
};

// function definition -------------------------------------
Program* parser();
#endif
