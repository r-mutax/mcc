#include "mcc.h"
#include "node.h"
#include "tokenize.h"

/*
    EBNF :

    expr = mul ( '+' mul | '-' mul )*
    mul = primary ( '*' primary | '/' primary )*
    primary = num | '(' expr ')'
    num = Integer.
*/


// local function forward declaration. ------------
static Node* new_node(NodeKind kind, Node* lhs, Node* rhs);
static Node* new_node_num(int val);
static Node* expr();
static Node* mul();
static Node* primary();

// -----------------------------------------------
Node* parser(){
    return expr();
}

Node* expr(){
    Node* node = mul();

    for(;;){
        if(tk_consume('+')){
            node = new_node(ND_ADD, node, mul());
        } else if(tk_consume('-')){
            node = new_node(ND_SUB, node, mul());
        } else {
            return node;
        }
    }
}

Node* mul(){
    Node* node = primary();
    
    for(;;){
        if(tk_consume('*')){
            node = new_node(ND_MUL, node, primary());
        } else if(tk_consume('/')){
            node = new_node(ND_DIV, node, primary());
        } else {
            return node;
        }
    }
}

Node* primary(){
    
    if(tk_consume('(')){
        Node* node = expr();
        tk_expect(')');
        return node;
    }

    Node* node = new_node_num(tk_expect_num());
    return node;
}

// local function definition ----------------------
Node* new_node(NodeKind kind, Node* lhs, Node* rhs){
    Node* node = calloc(1, sizeof(Node));

    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node* new_node_num(int val){
    Node* node = calloc(1, sizeof(Node));

    node->kind = ND_NUM;
    node->val = val;
    return node;
}