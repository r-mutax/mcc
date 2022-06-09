#include "mcc.h"
#include "node.h"
#include "tokenize.h"
#include "symtbl.h"
#include "type.h"
#include "scope.h"
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
            | 'switch' '(' expr ')' stmt
    declaration = decl_spec* declarator ";"
    decl_spec = type_spec
    type_spec = "long" | "int" | "char" | 'short' | struct_spec
    struct_spec = 'struct' ident? '{' struct_declar* '}'
    declarator = '*' * ( ident | ident '[' num ']') 
    expr = assign (',' assign)*

    assign = bitAnd ( '=' assign 
                        | '+=' assign
                        | '-=' assign
                        | '*=' assign
                        | '/=' assign
                        | '%=' assign )?
    logicOr = logicAnd ('||' logicAnd)*
    logicAnd = bitOr ('&&' bitOr)*
    bitOr = bitXor ( '|' bitXor )*
    bitXor = bitAnd ( '^' bitAnd )*
    bitAnd = equality ('&' equality)*
    equality = relational ( '==' relational | '!=' relational)*
    relational = add ( '<' add | '<=' add | '>' add | '>=' add)*
    add = mul ( '+' mul | '-' mul | '%' mul )*
    mul = primary ( '*' unary | '/' unary )*
    unary = postfix
            | '++' unary
            | '--' unary
            | 'sizeof' unary
            | ('+' | '-' | '*' | '&' ) postfix
    postfix = primary ('[' expr ']' | '++' | '--')?
    primary = num
             | ident ( '(' expr? ( ',' expr )* ')' )?
             | '(' expr ')'
             | string_literal
    num = Integer.
    string_literal = '"' chalacter* '"'
*/


// local function forward declaration. ------------
static Node* new_node(NodeKind kind, Node* lhs, Node* rhs);
static Node* new_node_num(int val);
static Node* new_node_add(Node* lhs, Node* rhs);
static Node* new_node_sub(Node* lhs, Node* rhs);
static Node* new_node_mul(Node* lhs, Node* rhs);
static Node* new_node_div(Node* lhs, Node* rhs);
static Node* new_node_mod(Node* lhs, Node* rhs);
static Node* new_var(Symbol* sym);
static Node* new_inc(Node* var);
static Node* new_dec(Node* var);
static Node* new_string_literal(Token* tok);
static int new_unique_no();
static Node* exchange_constant_expr(Node* expr);
static Node* copy_node(Node* node);
static Node* find_case_label(Node* body, Node** default_label);
static Program* program();
static Node* function();
static Node* compound_stmt();
static Node* declaration();
static Type* decl_spec(StorageClassKind* sck);
static Type* type_spec();
static Type* struct_spec();
static Type* type_suffix(Type* ty);
static Symbol* declare(Type* ty, StorageClassKind sck);
static Node* stmt();
static Node* expr();
static Node* logicOr();
static Node* logicAnd();
static Node* bitOr();
static Node* bitXor();
static Node* bitAnd();
static Node* assign();
static Node* equality();
static Node* relational();
static Node* add();
static Node* mul();
static Node* postfix();
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
            cur = cur->next;
        } else {
            declaration();
        }
    }

    prog->func_list = head.next;
    prog->string_list = st_get_string_list();
    prog->data_list = st_get_data_list();

    return prog;
}

static bool is_function(){

    Token* bkup_tok = tk_get_token();
    bool retval = false;

    StorageClassKind sck = SCK_NONE;
    Type* ty = decl_spec(&sck);

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

    StorageClassKind sck = SCK_NONE;
    Type* ty = decl_spec(&sck);
    Symbol* sym = declare(ty, sck);
    st_declare(sym);

    sym->is_func = true;
    func->func_sym = sym;

    sc_start_scope();

    tk_expect("(");

    if(!tk_consume(")")){
        // this function has parameter.
        if(!tk_istype()){
            error("not a type.\n");
        }
        
        Symbol head;
        Symbol* cur = &head;
        do{
            Type* ty = tk_consume_type();
            while(tk_consume("*"))
                ty = ty_pointer_to(ty);
            
            Token* ident_tok = tk_expect_ident();
            Symbol* sym = st_make_symbol(ident_tok, ty);
            st_declare(sym);

            cur = cur->next = st_copy_symbol(sym);
            func->paramnum++;
        } while(tk_consume(","));
        func->param = head.next;

        tk_expect(")");
    }

    func->body = compound_stmt();
    ty_add_type(func->body);

    func->stack_size = st_get_stacksize();
    sc_end_scope();

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
    sc_start_scope();
    while(!tk_consume("}") && cur){
        if(tk_istype()){
            cur->next = declaration();
        } else {
            cur->next = stmt();
        }
        cur = cur->next;        
    }
    sc_end_scope();
    // end block scope <--
    node->body = head.next;
    return node;
}

static Member* struct_member(){
    Member head;
    Member* cur = &head;
    do{
        cur = cur->next = calloc(1, sizeof(Member));
        StorageClassKind sck = SCK_NONE;
        Type* ty = decl_spec(&sck);
        cur->sym = declare(ty, sck);
        tk_expect(";");
    } while(!tk_consume("}"));

    return head.next;
}

static Type* struct_spec(){

    Token* tok = tk_consume_ident();

    if(tok){
        if(!tk_consume("{")){
            // registerd struct.
            Type* ty = ty_find_struct(tok->str, tok->len);

            if(!ty) error_at(tok->str, "implicit type.");
            return ty;
        }
    }
    tk_consume("{");
    sc_start_scope();
    // this point.
    // unnamed struct or named struct.
    Type* ty = calloc(1, sizeof(Type));
    ty->kind = TY_STRUCT;
    ty->member = struct_member();
    sc_end_scope();

    int offset = 0;
    for(Member* cur = ty->member; cur; cur = cur->next){
        cur->sym->offset = offset;
        offset += cur->sym->type->size;
    }
    ty->size = offset;

    if(tok){
        ty->name = calloc(1, tok->len + 1);
        strncpy(ty->name, tok->str, tok->len);
        ty->len = tok->len;
        ty_register_struct(ty);
    } else {
        ty->name = "__unnamed_struct";
    }
    return ty;
}

static Type* enum_spec(){
    Token* tok = tk_consume_ident();

    /*
        enum not-registerd-ident '{' enumelate-list '}'
        enum registerd-ident
        enum '{' enumelater-list '}'
    */

    if(tok){
        Type* ty = ty_find_enum(tok->str, tok->len);
        if(ty){
            return ty;
        }
    }

    if(!tk_consume("{"))
        error_at(tk_get_token()->str, "expect enumeration declare.\n");

    Type* ty = calloc(1, sizeof(Type));
    ty->kind = TY_ENUM;
    ty->size = 8;
    if(tok){
        ty->name = calloc(1, tok->len + 1);
        strncpy(ty->name, tok->str, tok->len);
        ty->len = tok->len;
        ty_register_struct(ty);
    } else {
        ty->name = "__unnamed_enum";
    }

    int val = -1;
    do {
        Token* ident = tk_expect_ident();
        if(!ident){
            break;
        }
        Symbol* sym = st_make_symbol(ident, ty);
        if(tk_consume("=")){
            Node* node = logicOr();
            node = exchange_constant_expr(node);
            val = node->val;
        } else {
            val++;
        }
        sym->enum_val = val;
        sym->is_enum_symbol = true;
        st_declare(sym);
    } while(tk_consume(","));
    tk_expect("}");

    return ty;
}

// check storage class specifiers.
// auto / register class is ignored in this compiler.
// if consumption token this function return true and dosen't return false.
static bool check_storage_class_keyword(StorageClassKind* sck, Token* tok){
    
    if(tk_consume_keyword("typedef")){
        if(*sck != SCK_NONE){
            error_at(tok->str, "multiple storage classes in declaration specifies.");
        }
        *sck = SCK_TYPEDEF;
        return true;
    }
    
    if(tk_consume_keyword("extern")){
        if(*sck != SCK_NONE){
            error_at(tok->str, "multiple storage classes in declaration specifies.");
        }
        *sck = SCK_EXTERN;
        return true;
    }

    if(tk_consume_keyword("static")){
        if(*sck != SCK_NONE){
            error_at(tok->str, "multiple storage classes in declaration specifies.");
        }
        *sck = SCK_STATIC;
        return true;
    }

    // these keyword is recognized but ignored.
    if(tk_consume_keyword("auto")
        || tk_consume_keyword("register"))
    {
        return true;
    }

   return false;
}

static void count_decl_spec(int* type_flg, int flg, Token* tok){

    // error check
    int target = 0;
    switch(flg){
        case K_LONG:
            // 'long' keyword can use up to 2 times in declaration.
            // ex) long long int
            target = *type_flg & (K_LONG << 1);
            break;
        default:
            target = *type_flg & flg;
            break;
    }

    if(target){
        error_at(tok->str, "duplicate type keyword.\n");
    }

    *type_flg += flg;
}

static Type* decl_spec(StorageClassKind* sck){

    // parse type specifies.
    int type_flg = 0;
    Type* ty = NULL;
    while(tk_istype()){
        Token* tok = tk_get_token();

        // check storage class keyword.
        if(check_storage_class_keyword(sck, tok)){
            continue;
        }

        // user type.
        if(tk_consume_keyword("struct")){
            if(type_flg)
                error_at(tok->str, "duplicate type keyword.\n");
            ty = struct_spec();
            type_flg += K_USER;
            continue;
        }

        // enumeration.
        if(tk_consume_keyword("enum")){
            if(type_flg)
                error_at(tok->str, "duplicate type keyword.\n");
            ty = enum_spec();
            type_flg += K_USER;
            continue;
        }

        // chcek built-in type keyword.
        if(tk_consume_keyword("void"))
            count_decl_spec(&type_flg, K_VOID, tok);
        if(tk_consume_keyword("_Bool"))
            count_decl_spec(&type_flg, K_BOOL, tok);
        if(tk_consume_keyword("char"))
            count_decl_spec(&type_flg, K_CHAR, tok);
        if(tk_consume_keyword("short"))
            count_decl_spec(&type_flg, K_SHORT, tok);
        if(tk_consume_keyword("int"))
            count_decl_spec(&type_flg, K_INT, tok);
        if(tk_consume_keyword("long"))
            count_decl_spec(&type_flg, K_LONG, tok);
        if(tk_consume_keyword("signed"))
            count_decl_spec(&type_flg, K_SIGNED, tok);
        if(tk_consume_keyword("unsigned"))
            count_decl_spec(&type_flg, K_UNSIGNED, tok);
    }

    if(!ty){
        switch(type_flg){
            case K_VOID:
                ty = ty_get_type("void", 4);
                break;
            case K_CHAR:
            case K_SIGNED + K_CHAR:
                ty = ty_get_type("char", 4);
                break;
            case K_SHORT:
            case K_SHORT + K_INT:
            case K_SIGNED + K_SHORT:
            case K_SIGNED + K_SHORT + K_INT:
                ty = ty_get_type("short", 5);
                break;
            case K_INT:
            case K_SIGNED:
            case K_SIGNED + K_INT:
                ty = ty_get_type("int", 3);
                break;
            case K_LONG:
            case K_LONG + K_INT:
            case K_LONG + K_LONG:
            case K_LONG + K_LONG + K_INT:
            case K_SIGNED + K_LONG:
            case K_SIGNED + K_LONG + K_INT:
            case K_SIGNED + K_LONG + K_LONG:
            case K_SIGNED + K_LONG + K_LONG + K_INT:
                ty = ty_get_type("long", 4);
                break;
            default:
                error_at(tk_get_token()->str, "Invalid type.\n");
                break;
        }
    }

    return ty;
}

static Node* declaration(){

    StorageClassKind sck = SCK_NONE;
    Type* ty = decl_spec(&sck);

    if(ty->kind == TY_STRUCT && tk_consume(";")){
        Node* node = calloc(1, sizeof(Node));
        node->kind = ND_CMPDSTMT;
        return node;
    }

    if(ty->kind == TY_ENUM && tk_consume(";")){
        Node* node = calloc(1, sizeof(Node));
        node->kind = ND_CMPDSTMT;
        return node;
    }

    Node head;
    memset(&head, 0, sizeof(Node));
    Node* cur = &head;

    do {
        Symbol* sym = declare(ty, sck);
        if(sym->type->kind == TY_VOID){
            error("declare variable with void type.\n");
        }

        st_declare(sym);
        if(sym->is_globalvar || sym->is_static){
            st_register_data(sym);
        } else {
            if(tk_consume("=")){
                cur->next = calloc(1, sizeof(Node));
                cur->next->kind = ND_ASSIGN;
                cur->next->lhs = new_var(sym);
                cur->next->rhs = assign();

                cur = cur->next;
            }
        }
    } while(tk_consume(","));
    tk_expect(";");

    if(&head.next){
        Node* node = calloc(1, sizeof(Node));
        node->kind = ND_CMPDSTMT;
        node->body = head.next;
        return node;
    }

    return NULL;
}

static Symbol* declare(Type* ty, StorageClassKind sck){
    while(tk_consume("*")){
        ty = ty_pointer_to(ty);
    }

    Token* tok = tk_expect_ident();
    ty = type_suffix(ty);

    Symbol* sym = st_make_symbol(tok, ty);
    
    switch(sck){
        case SCK_STATIC:
            sym->is_static = true;
            break;
        default:
            break;
    }
    return sym;
}

static Type* type_suffix(Type* ty){

    if(tk_consume("[")){
        int len = tk_expect_num();
        tk_expect("]");
        ty = type_suffix(ty);
        return ty_array_of(ty, len);
    }

    return ty;
}

static Node* exchange_constant_expr(Node* expr){

    if(expr->kind == ND_NUM){
        return expr;
    }

    expr->lhs = exchange_constant_expr(expr->lhs);
    expr->rhs = exchange_constant_expr(expr->rhs);
    
    int retval = 0;
    int lhs = expr->lhs->val;
    int rhs = expr->rhs->val;
    switch(expr->kind){
        case ND_ADD:
            retval = lhs + rhs;
            break;
        case ND_SUB:
            retval = lhs - rhs;
            break;
        case ND_MUL:
            retval = lhs * rhs;
            break;
        case ND_DIV:
            retval = lhs / rhs;
            break;
        case ND_MOD:
            retval = lhs % rhs;
            break;
        case ND_EQ:
            retval = lhs == rhs;
            break;
        case ND_NE:
            retval = lhs != rhs;
            break;
        case ND_LT:
            retval = lhs < rhs;
            break;
        case ND_LE:
            retval = lhs <= rhs;
            break;
        case ND_BIT_AND:
            retval = lhs & rhs;
            break;
        case ND_BIT_OR:
            retval = lhs | rhs;
            break;
        case ND_BIT_XOR:
            retval = lhs ^ rhs;
            break;
        case ND_AND:
            retval = lhs && rhs;
            break;
        case ND_OR:
            retval = lhs || rhs;
            break;
        default:
            error("case label can use only constant-expr.\n");
    }
    return new_node_num(retval);
}

static Node* find_case_label(Node* body, Node** default_label){
    Node* cur = body;
    Node case_head;
    Node* cur_case = &case_head;
    while(cur){
        switch(cur->kind){
            case ND_CASE:
                cur_case = cur_case->next = cur->lhs = exchange_constant_expr(cur->lhs);
                cur_case->val = cur->lhs->val;
                break;
            case ND_CMPDSTMT:
                cur_case->next = find_case_label(cur->body, default_label);
                while(cur_case->next){
                    cur_case = cur_case->next;
                }
                break;
            case ND_DEFAULT:
                if(*default_label)
                    error_at(cur->line, "default label depulicate.\n");
                *default_label = new_node(ND_DEFAULT, NULL, NULL);
                break;
        }
        cur = cur->next;
    }

    return case_head.next;
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
    } else if(tk_consume_keyword("switch")){
        node = new_node(ND_SWITCH, NULL, NULL);
        
        tk_expect("(");
        node->cond = expr();
        tk_expect(")");
        node->body = stmt();
        node->case_label = find_case_label(node->body, &node->default_label);
        return node;
    } else if(tk_consume_keyword("case")){
        node = new_node(ND_CASE, logicOr(), NULL);
        node->line = tk_get_token()->str;
        tk_expect(":");
        return node;
    } else if(tk_consume_keyword("default")){
        node = new_node(ND_DEFAULT, NULL, NULL);
        tk_expect(":");
        return node;
    } else if(tk_consume_keyword("break")){
        tk_expect(";");
        node = new_node(ND_BREAK, NULL, NULL);
        return node;
    } else if(tk_consume_keyword("goto")){
        Token* tok = tk_expect_ident();
        tk_expect(";");
        node = new_node(ND_GOTO, NULL, NULL);
        node->label_name = calloc(1, tok->len + 1);
        strncpy(node->label_name, tok->str, tok->len);
        return node;
    } else if(tk_is_label()){
        Token* tok = tk_consume_ident();
        tk_expect(":");
        node = new_node(ND_LABEL, NULL, NULL);
        node->label_name = calloc(1, tok->len + 1);
        strncpy(node->label_name, tok->str, tok->len);
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
    Node* node = assign();

    if (tk_consume(",")){
        node = new_node(ND_COMMA, node, expr());
    }

    return node;
}

static Node* assign(){
    Node* node = logicOr();

    if(tk_consume("=")){
        node = new_node(ND_ASSIGN, node, assign());
    } else if(tk_consume("+=")){
        node = new_node(ND_ASSIGN, node, new_node_add(node, assign()));
    } else if(tk_consume("-=")){
        node = new_node(ND_ASSIGN, node, new_node_sub(node, assign()));
    } else if(tk_consume("*=")){
        node = new_node(ND_ASSIGN, node, new_node_mul(node, assign()));
    } else if(tk_consume("/=")){
        node = new_node(ND_ASSIGN, node, new_node_div(node, assign()));
    } else if(tk_consume("%=")){
        node = new_node(ND_ASSIGN, node, new_node_mod(node, assign()));
    }
    return node;
}

static Node* logicOr(){
    Node* node = logicAnd();

    while(tk_consume("||")){
        node = new_node(ND_OR, node, logicAnd());
    }
    return node;
}
static Node* logicAnd(){
    Node* node = bitOr();
    for(;;){
        if(tk_consume("&&")){
            node = new_node(ND_AND, node, bitOr());
        } else {
            break;
        }
    }
    return node;
}

static Node* bitOr(){
    Node* node = bitXor();

    for(;;){
        if(tk_consume("|")){
            node = new_node(ND_BIT_OR, node, bitXor());
        } else {
            break;
        }
    }

    return node;
}

static Node* bitXor(){
    Node* node = bitAnd();

    for(;;){
        if(tk_consume("^")){
            node = new_node(ND_BIT_XOR, node, bitAnd());
        } else {
            break;
        }
    }

    return node;
}

static Node* bitAnd(){
    Node* node = equality();

    for(;;){
        if(tk_consume("&")){
            node = new_node(ND_BIT_AND, node, equality());
        } else {
            break;
        }
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
        return new_node_num(node->type->size);
    } else if(tk_consume("+")){
        return unary();
    } else if(tk_consume("-")){
        return new_node(ND_SUB, new_node_num(0), unary());
    } else if(tk_consume("&")){
        return new_node(ND_ADDR, unary(), NULL);
    } else if(tk_consume("*")){
        return new_node(ND_DREF, unary(), NULL);
    } else if(tk_consume("++")){
        // ++a -> a = a + 1
        Node* node = unary();
        return new_node(ND_ASSIGN, node, new_node_add(node, new_node_num(1)));
    } else if(tk_consume("--")){     
        // --a -> a = a - 1
        Node* node = unary();
        return new_node(ND_ASSIGN, node, new_node_sub(node, new_node_num(1)));
    } else {
        return postfix();
    }
}

static Node* postfix(){
    Node* node = primary();

    for(;;){
        if(tk_consume("[")){
        // array
            node = new_node(ND_DREF, new_node_add(node, expr()), NULL);
            tk_expect("]");
            ty_add_type(node);
            continue;
        }

        if(tk_consume("++")){
            node = new_inc(node);
            continue;
        }

        if(tk_consume("--")){
            node = new_dec(node);
            continue;
        }

        if(tk_consume("->")){
            node = new_node(ND_DREF, node, NULL);
            ty_add_type(node);
            node = new_node(ND_MEMBER, node,NULL);
            Token* tok = tk_expect_ident();
            Symbol* sym = ty_get_member(node->lhs->type, tok);
            if(sym == NULL){
                error_at(tok->str, "Not a member.\n");
            }
            node->type = sym->type;
            node->offset = sym->offset;
            continue;
        }

        if(tk_consume(".")){
            ty_add_type(node);
            node = new_node(ND_MEMBER, node,NULL);
            node->type = node->lhs->type;
            Token* tok = tk_expect_ident();
            Symbol* sym = ty_get_member(node->lhs->type, tok);
            if(sym == NULL){
                error_at(tok->str, "Not a member.\n");
            }
            node->type = sym->type;
            node->offset = sym->offset;
            continue;
        }
        
        return node;
    }
}

static Node* primary(){
    
    // expr ?
    if(tk_consume("(")){
        Node* node = expr();
        tk_expect(")");
        return node;
    }

    Token* tok;
    // string constant?
    tok = tk_consume_string();
    if(tok != NULL){
        Symbol* sym = st_register_literal(tok);
        Node* node = new_var(sym);

        if(tk_consume("[")){
            // array
            Node* node_deref = new_node(ND_DREF, new_node_add(node, expr()), NULL);
            tk_expect("]");
            return node_deref;
        }
        return node;;
    }

    // ident ?
    tok = tk_consume_ident();
    if(tok != NULL){
        Symbol* sym = st_find_symbol(tok);

        if (tk_consume("(")){
            Node* node = calloc(1, sizeof(Node));
            node->kind = ND_CALL;
            node->sym = sym;

            // has arguments?
            if(!tk_consume(")")){
                Node* arg_head = assign();
                Node* cur = arg_head;

                while(tk_consume(",")){
                    cur->next = assign();
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
                error_at(tok->str, "Not declared.\n");
            }

            Node* node;
            if(sym->is_enum_symbol){
                node = new_node_num(sym->enum_val);
                node->type = sym->type;
            } else {
                node = new_var(sym);
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
    if(sym->is_globalvar || sym->is_static){
        node->kind = ND_GVAR;
    } else {
        node->kind = ND_LVAR;
    }

    node->offset = sym->offset;
    node->type = sym->type;
    node->sym = sym;

    return node;
}

static Node* new_node_mul(Node* lhs, Node* rhs){
    ty_add_type(lhs);
    ty_add_type(rhs);

    Node* node = calloc(1, sizeof(Node));
    node->kind = ND_MUL;

    if(lhs->type->pointer_to || rhs->type->pointer_to){
        error("error : Try multiple pointer.\n");
    }

    node->lhs = lhs;
    node->rhs = rhs;    
    return node;
}

static Node* new_node_div(Node* lhs, Node* rhs){
    ty_add_type(lhs);
    ty_add_type(rhs);

    Node* node = calloc(1, sizeof(Node));
    node->kind = ND_DIV;

    if(lhs->type->pointer_to || rhs->type->pointer_to){
        error("error : Try division pointer.\n");
    }

    node->lhs = lhs;
    node->rhs = rhs;    
    return node;
}

static Node* new_node_mod(Node* lhs, Node* rhs){
    ty_add_type(lhs);
    ty_add_type(rhs);

    Node* node = calloc(1, sizeof(Node));
    node->kind = ND_MOD;

    if(lhs->type->pointer_to || rhs->type->pointer_to){
        error("error : Try division pointer.\n");
    }

    node->lhs = lhs;
    node->rhs = rhs;    
    return node;
}

// Type* tmp = x; x += 1; *tmp;
static Node* new_inc(Node* var){
    Token tok;
    tok.str = "tmp";
    tok.len = 3;
    tok.kind = TK_IDENT; 
    sc_start_scope();
    Type* ty = calloc(1, sizeof(Type));
    memcpy(ty, var->type, sizeof(Type));
    Symbol* tmp = st_make_symbol(&tok, ty);
    st_declare(tmp);

    Node* node_tmp = new_var(tmp);
    Node* node_assign = new_node(ND_ASSIGN, node_tmp, var);
    Node* node_inc = new_node(ND_ASSIGN, var, new_node_add(var, new_node_num(1)));
    Node* node = new_node(ND_COMMA, node_assign, new_node(ND_COMMA, node_inc, node_tmp));

    sc_end_scope();

    return node;
}

static Node* new_dec(Node* var){
    Token tok;
    tok.str = "tmp";
    tok.len = 3;
    tok.kind = TK_IDENT; 
    sc_start_scope();
    Type* ty = calloc(1, sizeof(Type));
    memcpy(ty, var->type, sizeof(Type));
    Symbol* tmp = st_make_symbol(&tok, ty);
    st_declare(tmp);

    Node* node_tmp = new_var(tmp);
    Node* node_assign = new_node(ND_ASSIGN, node_tmp, var);
    Node* node_inc = new_node(ND_ASSIGN, var, new_node_sub(var, new_node_num(1)));
    Node* node = new_node(ND_COMMA, node_assign, new_node(ND_COMMA, node_inc, node_tmp));

    sc_end_scope();

    return node;
}

static Node* copy_node(Node* node){
    Node* ret = calloc(1, sizeof(Node));
    memcpy(ret, node, sizeof(Node));
    ret->next = NULL;
}
