#ifndef MCPP_INC_H
#define MCPP_INC_H
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>


typedef struct SrcFile SrcFile;
typedef struct PP_Token PP_Token;
typedef struct Macro Macro;
typedef struct IF_SECTION_GROUP IF_SECTION_GROUP;

struct SrcFile{
    char* path;
    char* input_data;
    SrcFile* next;
};

typedef enum {
    PTK_NOTHING = 0,
    PTK_OPERAND,
    PTK_NUM,
    PTK_IDENT,
    PTK_PP_KEYWORD,
    PTK_KEYWORD,
    PTK_STRING_CONST,
    PTK_HASH,
    PTK_NEWLINE,
    PTK_SPACE,
    PTK_EOF
} PP_TokenKind;

struct PP_Token{
    PP_TokenKind    kind;
    PP_Token*       next;
    int             val;
    char*           str;
    int             len;

    char*           pos;
    long            row;
    SrcFile*        src;
};

typedef enum {
    PP_NONE = 0,
    PP_INCLUDE,
    PP_IF,
    PP_IFDEF,
    PP_IFNDEF,
    PP_ELIF,
    PP_ELSE,
    PP_ENDIF,
    PP_ERROR,
    PP_DEFINE,
    PP_DEFINED,
    PP_UNDEF
} PP_KIND;

struct Macro {
    PP_Token* def;
    PP_Token* val;
    PP_Token* param;
    bool is_func;
    Macro* next;
};

struct IF_SECTION_GROUP {
    int cond;
    PP_Token* head;
    PP_Token* tail;
    IF_SECTION_GROUP* next;
};

// -----------------------------------------------------------------

// tokenizer
PP_Token* ptk_tokenize_file(char* path);
PP_Token* new_token(PP_TokenKind kind, PP_Token* cur, char* str, int len);

// preprocessor
PP_Token* preprocess(PP_Token* tok);

// file io
char* read_file(char* path);
char* get_filename(SrcFile* src_file);

// errormsg
void error(char *fmt, ...);
void error_at(PP_Token* tok, char *msg);

// utility
char* strdup(char* src);
char* strndup(char* src, size_t n);

// dbeug
void print_token(PP_Token* tok);

#endif