#ifndef NODE_INC_H
#define NODE_INC_H

// parser data definition ----------------------------------
typedef enum {
    ND_ADD = 0,
    ND_SUB,
    ND_MUL,
    ND_DIV,
    ND_NUM
} NodeKind;

typedef struct Node Node;

struct Node {
    NodeKind    kind;
    Node*       lhs;
    Node*       rhs;
    int         val;
};

// function definition -------------------------------------
Node* parser();
#endif
