#ifndef MCC_INC_H
#define MCC_INC_H
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <errno.h>

typedef struct Token Token;
typedef struct Type Type;
typedef struct Node Node;
typedef struct Program Program;
typedef struct Function Function;
typedef struct Symbol Symbol;
typedef struct Member Member;

// token data definition --------------------------
typedef enum {
    TK_OPERAND = 0,
    TK_NUM,
    TK_IDENT,
    TK_KEYWORD,
    TK_STRING_CONST,
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
    ND_GVAR,
    ND_RETURN,
    ND_IF,
    ND_WHILE,
    ND_CMPDSTMT,
    ND_FOR,
    ND_CALL,
    ND_DECLARE,
    ND_FUNCTION,
    ND_COMMA,
    ND_BIT_AND,
    ND_BIT_OR,
    ND_BIT_XOR,
    ND_AND,
    ND_OR,
    ND_MEMBER
} NodeKind;

struct Node {
    NodeKind    kind;
    Node*       lhs;
    Node*       rhs;
    int         val;
    int         offset;
    Type*       type;

    Node*       next;

    Node*       cond;
    Node*       body;
    Node*       init;
    Node*       iter;
    Node*       else_body;
    Symbol*     sym;

    Node*       arg;

    char*       line;

    // function definition.
    Symbol*     func_sym;
    int         stack_size;
    Symbol*     param;
    int         paramnum;
    int         stacksize;
    bool        is_declare;
};

struct Program {
    Node*   func_list;
    Symbol* string_list;
};

// symbol table data definition ----------------------------------
struct Symbol{
    Symbol*     next;
    char*       name;
    int         offset;
    int         len;
    bool        is_func;
    bool        is_grobalvar;
    Type*       type;
    char*       str;
};

// type data definition --------------------------

struct Member {
    Symbol* sym;
    Member* next;
};

typedef enum {
    TY_INTEGER = 0,
    TY_POINTER,
    TY_ARRAY,
    TY_STRUCT
} TypeKind;

struct Type {
    char*       name;
    int         len;
    Type*       next;
    TypeKind    kind;
    Type*       pointer_from;
    Type*       pointer_to;
    
    Member*     member;

    int         array_len;

    int     size;
};

typedef struct Scope Scope;

typedef enum {
    SC_GROBAL = 0,
    SC_FUNCTION,
    SC_BLOCK
} ScopeKind;

struct Scope{
    ScopeKind   kind;
    Symbol*     symbol;
    int         stacksize;
    Scope*      child;
    Scope*      parent;
};

#endif