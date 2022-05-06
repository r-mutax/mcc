#include "mcc.h"
#include "node.h"
#include "tokenize.h"
#include "symtbl.h"
#include "errormsg.h"

/*
    EBNF :

    program = stmt*
    function = ident '(' ( ident ( ',' ident )* )? ')' compound_stmt
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
    primary = num
             | ident ( '(' expr? ( ',' expr )* ')' )?
             | '(' expr ')'
    num = Integer.
*/


// local function forward declaration. ------------
static Node* new_node(NodeKind kind, Node* lhs, Node* rhs);
static Node* new_node_num(int val);
static Program* program();
static Function* function();
static Node* compound_stmt();
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

    st_init();
    
    return program();
}

Program* program(){
    Program* prog = calloc(1, sizeof(Program));

    Function head;
    Function* cur = &head;

    while(!tk_iseof()){
        cur->next = function();
        cur = cur->next;
    }

    prog->func_list = head.next;
}

static Function* function(){
    
    Function* func = calloc(1, sizeof(Function));

    Token* tok = tk_expect_ident();
    func->name = tok->str;
    func->len = tok->len;
    
    st_start_scope();

    tk_expect("(");

    if(!tk_consume(")")){
        // this function has parameter.
        Token* ident_tok = tk_expect_ident();
        Symbol* sym = st_find_symbol(ident_tok);
        if(!sym){
            st_declare(ident_tok);
            sym = st_find_symbol(ident_tok);
        }
        func->param = sym;

        while(tk_consume(",")){
            ident_tok = tk_expect_ident();
            sym->next = st_find_symbol(ident_tok);
            if(!sym->next){
                st_declare(ident_tok);
                sym->next = st_find_symbol(ident_tok);
            }
            sym = sym->next;
        }
        tk_expect(")");
    }

    func->body = compound_stmt();

    func->stack_size = st_get_stacksize();
    st_end_scope();

    return func;
}

Node* compound_stmt(){
    if(!tk_consume("{")){
        return NULL;
    }

    Node* node = new_node(ND_CMPDSTMT, NULL, NULL);
    Node head;
    Node* cur = &head; 

    // start block scope. -->
    st_start_scope();
    while(!tk_consume("}") && cur){
        cur->next = stmt();
        cur = cur->next;
    }
    st_end_scope();
    // end block scope <--
    node->body = head.next;
    return node;
}

Node* stmt(){

    char* p = tk_getline();

    Node* cmpdstmt = compound_stmt();
    if(cmpdstmt){
        cmpdstmt->line = p;
        return cmpdstmt;
    }

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
        if(tk_consume_keyword("else")){
            node->else_body = stmt();
        }

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
    } else if(tk_consume_keyword("for")) {
        node = new_node(ND_FOR, NULL, NULL);

        tk_expect("(");
        node->init = expr();
        tk_expect(";");
        node->cond = expr();
        tk_expect(";");
        node->iter = expr();
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
        if (tk_consume("(")){
            Node* node = calloc(1, sizeof(Node));
            node->kind = ND_CALL;
            node->func_name = tok->str;
            node->len = tok->len;

            // has arguments?
            if(!tk_consume(")")){
                Node* arg_head = expr();
                Node* cur = arg_head;

                while(tk_consume(",")){
                    cur->next = expr();
                    cur = cur->next;
                }
                node->arg = arg_head;
                tk_expect(")");
                return node;
            } else {
                return node;
            }
        } else {
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