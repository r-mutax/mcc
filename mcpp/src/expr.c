#include "mcpp.h"

// token accessor
static PP_Token* token;
static void expect(char* op);
static bool consume(char* op);
static int expect_num();
static void skip_space();

// parsing
static int expr();
static int cond_expr();
static int logicOr();
static int logicAnd();
static int bitOr();
static int bitXor();
static int bitAnd();
static int equality();
static int relational();
static int bitShift();
static int add();
static int mul();
static int unary();
static int primary();

int constant_expr(PP_Token* tok){
    token = tok;
    return expr(tok);
}

static void expect(char* op){
    skip_space();
    if(!consume(op)){
        error_at(token, "[error] No expected tokens.\n");
    }
}

static bool consume(char* op){
    skip_space();
    if(strlen(op) == token->len
        && memcmp(op, token->str, token->len) == 0
        && token->kind == PTK_OPERAND){
        token = token->next;
        return true;
    }
    return false;
}

static int expect_num(){
    skip_space();
    if(token->kind != PTK_NUM)
        error_at(token, "No expected tokens.\n");

    int ans = token->val;
    token = token->next;
    return ans;  
}

static void skip_space(){
    if(token->kind == PTK_SPACE)
        token = token->next;
}

static int expr(){
    return cond_expr();
}

static int cond_expr(){
    int ans = logicOr();

    if(consume("?")){
        int tpath = expr();
        expect(":");
        int fpath = expr();

        ans = ans ? tpath : fpath;
    }
    return ans;
}

static int logicOr(){
    int ans = logicAnd();
    for(;;){
        if(consume("||")){
            ans = ans || logicAnd();
        } else {
            return ans;
        }
    }
}

static int logicAnd(){
    int ans = bitOr();
    for(;;){
        if(consume("&&")){
            ans = ans && bitOr();
        } else {
            return ans;
        }
    }
}

static int bitOr(){
    int ans = bitXor();
    for(;;){
        if(consume("|")){
            ans |= bitXor();
        } else {
            return ans;
        }
    }
}

static int bitXor(){
    int ans = bitAnd();
    for(;;){
        if(consume("^")){
            ans ^= bitAnd();
        } else {
            return ans;
        }
    }
}

static int bitAnd(){
    int ans = equality();
    for(;;){
        if(consume("&")){
            ans &= equality();
        } else {
            return ans;
        }
    }
}

static int equality(){
    int ans = relational();
    for(;;){
        if(consume("==")){
            ans = ans == relational();
        } else if(consume("!=")){
            ans = ans != relational();
        } else {
            return ans;
        }
    }
}

static int relational(){
    int ans = bitShift();
    for(;;){
        if(consume("<")){
            ans = ans < bitShift();
        } else if(consume("<=")){
            ans = ans <= bitShift();
        } else if(consume(">")){
            ans = ans > bitShift();
        } else if(consume(">=")){
            ans = ans >= bitShift();
        } else {
            return ans;
        }
    }
}

static int bitShift(){
    int ans = add();
    for(;;){
        if(consume("<<")){
            ans <<= add();
        } else if(consume(">>")){
            ans >>= add();
        } else {
            return ans;
        }
    }
}

static int add(){
    int ans = mul();
    for(;;){
        if(consume("+")){
            ans += mul();
        } else if(consume("-")){
            ans -= mul();
        } else {
            return ans;
        }
    }
}

static int mul(){
    int ans = unary();
    for(;;){
        if(consume("*")){
            ans *= unary();
        } else if(consume("/")){
            ans /= unary();
        } else if(consume("%")){
            ans %= unary();
        } else {
            return ans;
        }
    }
}

static int unary(){
    if(consume("+")){
        return primary();
    } else if(consume("-")){
        return -primary();
    } else if(consume("!")){
        return !primary();
    } else {
        return primary();
    }
}

static int primary(){
    if(consume("(")){
        int ans = expr();
        expect(")");
        return ans;
    } else {
        return expect_num();
    }
}