#include "mcc.h"
#include "node.h"
#include "tokenize.h"
#include "symtbl.h"
#include "errormsg.h"

/*
    EBNF :

    program = stmt*
    compound_stmt = '{' stmt* '}'
    stmt = expr ';'
            | 'return' expr ';'
            | 'if' '(' expr ')' stmt
            | 'while' '(' expr ')' ( stmt )
    expr = assign
    assign = equality ( '=' assign )?
    equality = relational ( '==' relational | '!=' relational)*
    relational = add ( '<' add | '<=' add | '>' add | '>=' add)*
    add = mul ( '+' mul | '-' mul )*
    mul = primary ( '*' unary | '/' unary )*
    unary = ('+' | '-')? primary
    primary = num | ident | '(' expr ')'
    num = Integer.
*/


// local function forward declaration. ------------
static Node* new_node(NodeKind kind, Node* lhs, Node* rhs);
static Node* new_node_num(int val);
static Program* program();
static Node* stmt();
static Node* expr();
static Node* assign();
static Node* equality();
static Node* relational();
static Node* add();
static Node* mul();
static Node* primary();
static Node* unary();

// -----------------------------------------------
Program* parser(){
    return program();
}

Program* program(){
    Program* prog = calloc(1, sizeof(Program));

    Node  head;
    Node* cur = &head;

    while(!tk_iseof()){
        cur->next = stmt();
        cur = cur->next;
    }

    prog->body = head.next;
}

Node* stmt(){

    char* p = tk_getline();

    Node* node = NULL;
    if(tk_consume_keyword("return")){
        node = new_node(ND_RETURN, expr(), NULL);
        tk_expect(";");
        node->line = p;
        return node;
    } else if(tk_consume_keyword("if")) {
        node = new_node(ND_IF, NULL, NULL);

        tk_expect("(");
        node->cond = expr();
        tk_expect(")");
        node->body = stmt();
        node->line = p;
        return node;
    } else if(tk_consume_keyword("while")) {
        node = new_node(ND_WHILE, NULL, NULL);
        
        tk_expect("(");
        node->cond = expr();
        tk_expect(")");
        node->body = stmt();
        node->line = p;
        return node;
    } else {
        node = expr();
        tk_expect(";");
        node->line = p;
        return node;
    }

    error("Expect Statement but get another node.\n");
    return NULL;
}

Node* expr(){
    return assign();
}

Node* assign(){
    Node* node = equality();

    if(tk_consume("=")){
        node = new_node(ND_ASSIGN, node, assign());
    }
    return node;
}

Node* equality(){
    Node* node = relational();

    for(;;){
        if(tk_consume("==")){
            node = new_node(ND_EQ, node, relational());
        } else if(tk_consume("!=")){
            node = new_node(ND_NE, node, relational());
        } else {
            return node;
        }
    }
}

Node* relational(){
    Node* node = add();

    for(;;){
        if(tk_consume("<")){
            node = new_node(ND_LT, node, add());
        } else if(tk_consume("<=")){
            node = new_node(ND_LE, node, add());
        } else if(tk_consume(">")){
            node = new_node(ND_LT, add(), node);
        } else if(tk_consume(">=")){
            node = new_node(ND_LE, add(), node);
        } else {
            return node;
        }
    }
}


Node* add(){
    Node* node = mul();

    for(;;){
        if(tk_consume("+")){
            node = new_node(ND_ADD, node, mul());
        } else if(tk_consume("-")){
            node = new_node(ND_SUB, node, mul());
        } else {
            return node;
        }
    }
}

Node* mul(){
    Node* node = unary();
    
    for(;;){
        if(tk_consume("*")){
            node = new_node(ND_MUL, node, unary());
        } else if(tk_consume("/")){
            node = new_node(ND_DIV, node, unary());
        } else {
            return node;
        }
    }
}

Node* unary(){
    
    if(tk_consume("+")){
        return primary();
    } else if(tk_consume("-")){
        return new_node(ND_SUB, new_node_num(0), primary());
    } else {
        return primary();
    }
}

Node* primary(){
    
    // expr ?
    if(tk_consume("(")){
        Node* node = expr();
        tk_expect(")");
        return node;
    }

    // ident ?
    Token* tok = tk_consume_ident();
    if(tok != NULL){

        Symbol* sym = st_find_symbol(tok);
        if(sym == NULL){
            st_declare(tok);
            sym = st_find_symbol(tok);
        }

        Node* node = calloc(1, sizeof(Node));
        node->kind = ND_LVAR;
        node->offset = sym->offset;
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