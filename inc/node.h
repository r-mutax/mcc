#ifndef NODE_INC_H
#define NODE_INC_H

// parser data definition ----------------------------------
typedef enum {
    ND_ADD = 0,
    ND_SUB,
    ND_MUL,
    ND_DIV,
    ND_NUM,
    ND_EQ,
    ND_NE,
    ND_LT,
    ND_LE,
    ND_ASSIGN,
    ND_LVAR
} NodeKind;

typedef struct Node Node;
typedef struct Program Program;

struct Node {
    NodeKind    kind;
    Node*       lhs;
    Node*       rhs;
    int         val;
    int         offset;

    Node*       next;

    char*       line;
};

struct Program {
    Node*       body;
};

// function definition -------------------------------------
Program* parser();
#endif