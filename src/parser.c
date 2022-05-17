#include "mcc.h"
#include "node.h"
#include "tokenize.h"
#include "symtbl.h"
#include "type.h"
#include "errormsg.h"

/*
    EBNF :

    program = ( function | declaration )*
    function = declspec* ident '(' ( ident ( ',' ident )* )? ')' compound_stmt
    compound_stmt = '{' ( stmt | declare )* '}'
    stmt = expr ';'
            | 'return' expr ';'
            | 'if' '(' expr ')' stmt 
            | 'while' '(' expr ')' ( stmt )
    declaration = decl_spec* declarator ";"
    decl_spec = type_spec
    type_spec = "long"
    declarator = '*' * ( ident | ident '[' num ']') 
    expr = assign
    assign = equality ( '=' assign )?
    equality = relational ( '==' relational | '!=' relational)*
    relational = add ( '<' add | '<=' add | '>' add | '>=' add)*
    add = mul ( '+' mul | '-' mul | '%' mul )*
    mul = primary ( '*' unary | '/' unary )*
    unary = ('+' | '-')? primary | ( '*' | '/' ) primary
    primary = num
             | ident ( '(' expr? ( ',' expr )* ')' )?
             | '(' expr ')'
    num = Integer.
*/


// local function forward declaration. ------------
static Node* new_node(NodeKind kind, Node* lhs, Node* rhs);
static Node* new_node_num(int val);
static Node* new_node_add(Node* lhs, Node* rhs);
static Node* new_node_sub(Node* lhs, Node* rhs);
static Node* new_var(Symbol* sym);
static Program* program();
static Node* function();
static Node* compound_stmt();
static Node* declaration();
static Type* decl_spec();
static Type* type_spec();
static Symbol* declare(Type* ty);
static Node* stmt();
static Node* expr();
static Node* assign();
static Node* equality();
static Node* relational();
static Node* add();
static Node* mul();
static Node* primary();
static Node* unary();
static bool is_function();

// -----------------------------------------------
Program* parser(){

    st_init();
    ty_init();
    
    return program();
}

static Program* program(){
    Program* prog = calloc(1, sizeof(Program));

    Node head;
    Node* cur = &head;

    while(!tk_iseof()){
        if(is_function()){
            cur->next = function();
        } else {
            cur->next = declaration();
        }
        cur = cur->next;
    }

    prog->func_list = head.next;
}

static bool is_function(){

    Token* bkup_tok = tk_get_token();
    bool retval = false;

    Type* ty = decl_spec();

    while(tk_consume("*")){
        ;
    }

    tk_expect_ident();
    if(tk_consume("(")){
        retval = true;
    }

    tk_set_token(bkup_tok);
    return retval;
}

static Node* function(){
    
    Node* func = calloc(1, sizeof(Node));
    func->kind = ND_FUNCTION;

    Type* ty = decl_spec();
    Symbol* sym = declare(ty);
    sym->is_func = true;
    func->func_sym = sym;

    st_start_scope();

    tk_expect("(");

    if(!tk_consume(")")){
        // this function has parameter.
        if(!tk_istype()){
            error("not a type.\n");
        }

        Type* ty = tk_consume_type();
        Token* ident_tok = tk_expect_ident();
        Symbol* sym = st_declare(ident_tok, ty);
        func->param = st_copy_symbol(sym);
        func->paramnum++;

        Symbol* cur = func->param;

        while(tk_consume(",")){

            if(!tk_istype()){
                error("not a type.\n");
            }

            ty = tk_consume_type();
            ident_tok = tk_expect_ident();
            sym = st_declare(ident_tok, ty);
            
            cur->next = st_copy_symbol(sym);
            cur = cur->next;
            func->paramnum++;
        }
        tk_expect(")");
    }

    func->body = compound_stmt();
    ty_add_type(func->body);

    func->stack_size = st_get_stacksize();
    st_end_scope();

    if(func->body == NULL){
        func->is_declare = true;
        tk_expect(";");
    }

    return func;
}

static Node* compound_stmt(){
    if(!tk_consume("{")){
        return NULL;
    }

    Node* node = new_node(ND_CMPDSTMT, NULL, NULL);
    Node head;
    Node* cur = &head; 

    // start block scope. -->
    st_start_scope();
    while(!tk_consume("}") && cur){
        if(tk_istype()){
            cur->next = declaration();
        } else {
            cur->next = stmt();
        }
        cur = cur->next;        
    }
    st_end_scope();
    // end block scope <--
    node->body = head.next;
    return node;
}

static Type* type_spec(){
    if(tk_consume_keyword("long")){
        return ty_get_type("long", 4);
    } else if(tk_consume_keyword("int")){
        return ty_get_type("int", 3);
    } else if(tk_consume_keyword("char")){
        return ty_get_type("char", 4);
    }

    return NULL;
}

static Type* decl_spec(){
    
    Type* ty = type_spec();
    if(ty == NULL){
        error("expect type.\n");
    }

    return ty;
}

static Node* declaration(){

    Type* ty = decl_spec();

    Node head = {};
    Node* cur = &head;

    do {
        Symbol* sym = declare(ty);
        if(sym->is_grobalvar){
            cur->next = calloc(1, sizeof(Node));
            cur->next->kind = ND_DECLARE;
            cur->next->sym = sym;
            cur = cur->next;
        } else {
            if(tk_consume("=")){
                cur->next = calloc(1, sizeof(Node));
                cur->next->kind = ND_ASSIGN;
                cur->next->lhs = new_var(sym);
                cur->next->rhs = expr();

                cur = cur->next;
            }
        }
    } while(tk_consume(","));
    tk_expect(";");

    Node* node = calloc(1, sizeof(Node));
    node->kind = ND_CMPDSTMT;
    node->body = head.next;

    return node;
}

static Symbol* declare(Type* ty){
    while(tk_consume("*")){
        ty = ty_pointer_to(ty);
    }

    Token* tok = tk_expect_ident();

    if(tk_consume("[")){
        int len = tk_expect_num();
        ty = ty_array_of(ty, len);
        tk_expect("]");
    }

    Symbol* sym = st_declare(tok, ty);
    return sym;
}

static Node* stmt(){

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

static Node* expr(){
    return assign();
}

static Node* assign(){
    Node* node = equality();

    if(tk_consume("=")){
        node = new_node(ND_ASSIGN, node, assign());
    }
    return node;
}

static Node* equality(){
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

static Node* relational(){
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


static Node* add(){
    Node* node = mul();

    for(;;){
        if(tk_consume("+")){
            node = new_node_add(node, mul());
        } else if(tk_consume("-")){
            node = new_node_sub(node, mul());
        } else {
            return node;
        }
    }
}

static Node* mul(){
    Node* node = unary();
    
    for(;;){
        if(tk_consume("*")){
            node = new_node(ND_MUL, node, unary());
        } else if(tk_consume("/")){
            node = new_node(ND_DIV, node, unary());
        } else if(tk_consume("%")) {
            node = new_node(ND_MOD, node, unary());
        } else {
            return node;
        }
    }
}

static Node* unary(){
    
    if(tk_consume_keyword("sizeof")){
        Node* node = unary();
        ty_add_type(node);

        if(node->type->kind == TY_ARRAY){
            return new_node_num(node->type->size * node->type->array_len);
        } else {
            return new_node_num(node->type->size);
        }
    } else if(tk_consume("+")){
        return primary();
    } else if(tk_consume("-")){
        return new_node(ND_SUB, new_node_num(0), primary());
    } else if(tk_consume("&")){
        return new_node(ND_ADDR, primary(), NULL);
    } else if(tk_consume("*")){
        return new_node(ND_DREF, primary(), NULL);
    } else {
        return primary();
    }
}

static Node* primary(){
    
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

        if (tk_consume("(")){
            Node* node = calloc(1, sizeof(Node));
            node->kind = ND_CALL;
            node->sym = sym;

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
            if(sym == NULL){
                error_at(tok->str, "%s is not declared.\n", tok->str);
            }

            Node* node = new_var(sym);
            if(tk_consume("[")){
                // array
                Node* node_deref = new_node(ND_DREF, new_node_add(node, expr()), NULL);
                tk_expect("]");
                return node_deref;
            }

            return node;
        }
    }

    Node* node = new_node_num(tk_expect_num());
    return node;
}

// local function definition ----------------------
static Node* new_node(NodeKind kind, Node* lhs, Node* rhs){
    Node* node = calloc(1, sizeof(Node));

    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

static Node* new_node_num(int val){
    Node* node = calloc(1, sizeof(Node));

    node->kind = ND_NUM;
    node->val = val;
    return node;
}

static Node* new_node_add(Node* lhs, Node* rhs){

    ty_add_type(lhs);
    ty_add_type(rhs);

    Node* node = calloc(1, sizeof(Node));
    node->kind = ND_ADD;

    // num + num
    if(!lhs->type->pointer_to
        && !rhs->type->pointer_to){
            node->lhs = lhs;
            node->rhs = rhs;
            return node;
        }

    // pointer + pointer
    if(lhs->type->pointer_to && rhs->type->pointer_to){
        error("Try add pointer and pointer.");
    }

    // pointer + num
    if(!lhs->type->pointer_to && rhs->type->pointer_to){
        Node* buf = lhs;
        lhs = rhs;
        rhs = lhs;
    }

    Node* node_mul = calloc(1, sizeof(Node));
    node_mul->kind = ND_MUL;
    node_mul->lhs = rhs;
    node_mul->rhs = new_node_num(lhs->type->pointer_to->size);

    node->lhs = lhs;
    node->rhs = node_mul;

    return node;
}

static Node* new_node_sub(Node* lhs, Node* rhs){
    
    ty_add_type(lhs);
    ty_add_type(rhs);

    Node* node = calloc(1, sizeof(Node));
    node->kind = ND_SUB;

    // num - num
    if(!lhs->type->pointer_to && !rhs->type->pointer_to){
        node->lhs = lhs;
        node->rhs = rhs;
        return node;
    }

    // pointer - pointer
    // num - pointer
    if(rhs->type->pointer_to){
        error("Try subtraction pointer\n");
    }

    // pointer - num
    if(lhs->type->pointer_to && !rhs->type->pointer_to){
        Node* node_mul = calloc(1, sizeof(Node));
        node_mul->kind = ND_MUL;
        node_mul->lhs = rhs;
        node_mul->rhs = new_node_num(lhs->type->pointer_to->size);

        node->lhs = lhs;
        node->rhs = node_mul;

        return node;
    }
}

static Node* new_var(Symbol* sym){

    Node* node = calloc(1, sizeof(Node));
    if(sym->is_grobalvar){
        node->kind = ND_GVAR;
    } else {
        node->kind = ND_LVAR;
    }

    node->offset = sym->offset;
    node->type = sym->type;
    node->sym = sym;

    return node;
}