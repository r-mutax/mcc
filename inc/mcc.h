#ifndef MCC_INC_H
#define MCC_INC_H
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>

typedef struct Token Token;
typedef struct Type Type;
typedef struct Node Node;
typedef struct Program Program;
typedef struct Function Function;
typedef struct Symbol Symbol;

// token data definition --------------------------
typedef enum {
    TK_OPERAND = 0,
    TK_NUM,
    TK_IDENT,
    TK_KEYWORD,
    TK_EOF
} TokenKind;

struct Token { 
    TokenKind   kind;
    Token*      next;
    int         val;
    char*       str;
    int         len;
};

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

// symbol table data definition ----------------------------------
struct Symbol{
    Symbol*     next;
    char*       name;
    int         len;
    int         offset;
    Type*       type;
};

// type data definition --------------------------

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

#endif