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
typedef struct SrcFile SrcFile;
typedef struct IncDir IncDir;
typedef struct Macro Macro;

// Macro definition -------------------------------
struct Macro{
    Token* def;
    Token* val;
    Macro* next;
};

// directory definition ---------------------------
struct IncDir {
    char* path;
    IncDir* next;
};

// sorce file definition --------------------------
struct SrcFile{
    char* path;
    char* input_data;
    SrcFile* next;
};

// token data definition --------------------------
typedef enum {
    TK_OPERAND = 0,
    TK_NUM,
    TK_IDENT,
    TK_KEYWORD,
    TK_STRING_CONST,
    TK_NEWLINE,
    TK_PREPROCESS,
    TK_EOF
} TokenKind;

struct Token { 
    TokenKind   kind;
    Token*      next;
    int         val;
    char*       str;
    int         len;
    SrcFile*    src;
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
    ND_DOWHILE,
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
    ND_COND_EXPR,
    ND_MEMBER,
    ND_LABEL,
    ND_GOTO,
    ND_SWITCH,
    ND_CASE,
    ND_BREAK,
    ND_CONTINUE,
    ND_DEFAULT,
    ND_VOID_STMT,
    ND_CAST,
    ND_BIT_LSHIFT,
    ND_BIT_RSHIFT
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
    char*       label_name;
    Node*       case_label;
    Node*       default_label;

    Node*       arg;

    Token*      line;

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
    Symbol* data_list;
};

// symbol table data definition ----------------------------------
struct Symbol{
    Symbol*     next;
    char*       name;
    char*       func_name;
    int         offset;
    int         len;
    bool        is_func;
    bool        is_globalvar;
    bool        is_static;
    bool        is_enum_symbol;
    int         enum_val;
    Type*       type;
    char*       str;
    
    Symbol*     args;
    int         argnum;
};

// type data definition --------------------------

struct Member {
    Symbol* sym;
    Member* next;
};

typedef enum {
    SCK_NONE = 0,
    SCK_TYPEDEF,
    SCK_EXTERN,
    SCK_STATIC,
    SCK_AUTO,
    SCK_REGISTER
} StorageClassKind;

typedef enum {
    TY_INTEGER = 0,
    TY_POINTER,
    TY_ARRAY,
    TY_VOID,
    TY_STRUCT,
    TY_ENUM
} TypeKind;

struct Type {
    char*       name;
    int         len;
    Type*       next;
    TypeKind    kind;
    Type*       pointer_from;
    Type*       pointer_to;
    bool        is_typedef;
    Type*       base_type;
    
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
    Type*       type;
    int         stacksize;
    Scope*      child;
    Scope*      parent;
};

enum {
    K_VOID      = 1 << 0,
    K_BOOL      = 1 << 2,
    K_CHAR      = 1 << 4,
    K_SHORT     = 1 << 6, 
    K_INT       = 1 << 8,
    K_LONG      = 1 << 10,      // up to 2 times can.
    K_USER      = 1 << 16,
    K_SIGNED    = 1 << 17,
    K_UNSIGNED  = 1 << 18
};

typedef enum {
    ierr = -1,
    i8 = 0,
    i16,
    i32,
    i64
} SIZE_TYPE_ID;

typedef enum {
    NO_OPTION = 0,
    INCLUDE_PATH,
    OUTPUT_FILE,
    C_SRC_FILE
} MCC_OPTION;

#endif