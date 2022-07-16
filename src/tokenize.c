#include "mcc.h"
#include "tokenize.h"
#include "type.h"
#include "errormsg.h"
#include "preprocessor.h"
#include "file.h"
#include "utility.h"

Token* token;
SrcFile* cur_file;

static long row = 1;

// local function forward definition. ------------
static Token* new_token(TokenKind kind, Token* cur, char* str, int len);
static bool startswith(char* lhs, char* rhs);
static bool is_ident1(char p);
static bool is_ident2(char p);
static bool is_keyword(char* lhs, char* rhs);
static bool check_keyword(char* keyword, char** p, Token** tok);
static bool check_preprocess(char* directive, char** p, Token** tok);
// -----------------------------------------------
Token* tk_tokenize_file(char* path){

    SrcFile* srcfile = calloc(1, sizeof(SrcFile));
    srcfile->input_data = read_file(path);
    srcfile->path = strdup(path);

    cur_file = srcfile;

    Token* tok = tk_tokenize(srcfile->input_data);
    tok = preprocess(tok);

    return tok;
}

Token* tk_tokenize(char* p){

    Token head;
    head.next = NULL;
    Token* cur = &head;

    while(*p){

        if(startswith(p, "\r\n")){
            cur = new_token(TK_NEWLINE, cur, p, 2);
            p += 2;
            continue;
        }

        if(*p == '\r' || *p == '\n'){
            cur = new_token(TK_NEWLINE, cur, p++, 1);
            continue;
        }

        // skip space, new line carrige return and so on.
        if(isspace(*p)){
            p++;
            continue;
        }

        if(strncmp(p, "//", 2) == 0){
            p += 2;
            while(*p != '\n'){
                p++;
            }
            continue;
        }

        if(strncmp(p, "/*", 2) == 0){
            char* q = p;
            while(*q){
                if(*q == '*' && *(q+1) == '/') break;
                if(*q == '\n')
                    row++;
                q++;
            }
            if(q == 0){
                error("error : Not close block comment.\n");
            }
            p = q + 2;
            continue;
        }

        if(*p == '"'){
            char* start = ++p;
            while(*p != '"'){
                p++;
            }
            cur = new_token(TK_STRING_CONST, cur, start, p - start);
            p++;
            continue;
        }

        if(*p == '\''){
            char a = *(++p);
            cur = new_token(TK_NUM, cur, p, 1);
            cur->val = a;

            while(*p != '\''){
                p++;
            }
            p++;
            continue;
        }

        if(startswith(p, "<<=")
            || startswith(p, ">>="))
        {
            cur = new_token(TK_OPERAND, cur, p, 3);
            p += 3;
            continue;
        }

        if(startswith(p, "==")
            || startswith(p, "!=")
            || startswith(p, "<=")
            || startswith(p, ">=")
            || startswith(p, "+=")
            || startswith(p, "-=")
            || startswith(p, "*=")
            || startswith(p, "/=")
            || startswith(p, "%=")
            || startswith(p, "<<")
            || startswith(p, ">>")
            || startswith(p, "++")
            || startswith(p, "--")
            || startswith(p, "&&")
            || startswith(p, "||")
            || startswith(p, "->"))
        {
            cur = new_token(TK_OPERAND, cur, p, 2);
            p += 2;
            continue;
        }

        if(strchr("+-*/,()<>:;={}&|^[].?%", *p)){
            cur = new_token(TK_OPERAND, cur, p++, 1);
            continue;
        }

        if(check_preprocess("#include", &p, &cur)
            || check_preprocess("#ifdef", &p, &cur)
            || check_preprocess("#ifndef", &p, &cur)
            || check_preprocess("#else", &p, &cur)
            || check_preprocess("#endif", &p, &cur)
            || check_preprocess("#define", &p, &cur)){

            continue;
        }

        if(check_keyword("return", &p, &cur)
            || check_keyword("if", &p, &cur)
            || check_keyword("else", &p, &cur)
            || check_keyword("for", &p, &cur)
            || check_keyword("while", &p, &cur)
            || check_keyword("do", &p, &cur)
            || check_keyword("goto", &p, &cur)
            || check_keyword("continue", &p, &cur)
            || check_keyword("switch", &p, &cur)
            || check_keyword("case", &p, &cur)
            || check_keyword("default", &p, &cur)
            || check_keyword("break", &p, &cur)
            || check_keyword("sizeof", &p, &cur)
            || check_keyword("int", &p, &cur)
            || check_keyword("char", &p, &cur)
            || check_keyword("long", &p, &cur)
            || check_keyword("short", &p, &cur)
            || check_keyword("void", &p, &cur)
            || check_keyword("static", &p, &cur)
            || check_keyword("typedef", &p, &cur)
            || check_keyword("enum", &p, &cur)
            || check_keyword("struct", &p, &cur)){
            continue;
        }

        if(is_ident1(*p)){
            char* start = p;
            p++;
            while(is_ident2(*p)){
                p++;
            }

            cur = new_token(TK_IDENT, cur, start, p - start);
            continue;
        }

        if('a' <= *p && *p <= 'z'){
            cur = new_token(TK_IDENT, cur, p++, 1);
            continue;
        }

        if(isdigit(*p)){
            cur = new_token(TK_NUM, cur, p, 0);
            char* q = p;
            cur->val = strtol(p, &p, 10);
            cur->len = p - q;
            continue;
        }

        error("find cannot tokenize words.\n");
    }

    new_token(TK_EOF, cur, p, 0);
    token = head.next;
    return head.next;
}

bool tk_iseof(){
    return (token->kind == TK_EOF);
}

void tk_expect(char* p){
    if(token->kind != TK_OPERAND
        || token->len != strlen(p)
        || memcmp(p, token->str, token->len))
    {
        char msg[256];
        sprintf(msg, "expect %s, but get %s\n", p, token->str);
        error_at(token, msg);
    }
    token = token->next;
}

int tk_expect_num(){
    if(token->kind != TK_NUM){
        error_at(token, "expect number.\n");
    }
    int ret = token->val;
    token = token->next;
    return ret;
}

bool tk_consume(char* op) {
    if (token->kind != TK_OPERAND
         || token->len != strlen(op)
         || memcmp(op, token->str, token->len))
        return false;
    token = token->next;
    return true;
}

Token* tk_consume_ident(){
    if(token->kind != TK_IDENT){
        return NULL;
    }

    Token* tok = token;
    token = token->next;
    return tok;
}

Token*  tk_expect_ident(){
    if(token->kind != TK_IDENT){
        error_at(token, "Expect Identifier, but get another token.\n");
    }

    Token* tok = token;
    token = token->next;
    return tok;
}

char* tk_getline(){
    return token->str;
}

bool tk_consume_keyword(char* keyword){
    if(token->kind != TK_KEYWORD
        || token->len != strlen(keyword)
        || memcmp(keyword, token->str, token->len)){
        return false;
    }
    token = token->next;
    return true;
}

void tk_expect_keyword(char* keyword){
    if(token->kind != TK_KEYWORD
        || token->len != strlen(keyword)
        || memcmp(keyword, token->str, token->len)){
        error_at(token, "Expect keyword, but get another token.\n");
    }
    token = token->next;
    return;
}

bool tk_istype(){
    Type* ty = ty_get_type(token->str, token->len);

    if(ty != NULL
        || (memcmp(token->str, "static", 6) == 0 && token->len == 6)
        || (memcmp(token->str, "typedef", 7) == 0 && token->len == 7)
        || (memcmp(token->str, "enum", 4) == 0 && token->len == 4)
        || (memcmp(token->str, "struct", 6) == 0 && token->len == 6)){
            return true;
    }
    return false;
}

Type* tk_consume_type(){
    Type* ty = ty_get_type(token->str, token->len);
    
    if(ty != NULL)
        token = token->next;
    return ty;
}

Type* tk_expect_type(){
    Type* ty = ty_get_type(token->str, token->len);
    if(ty == NULL){
        error_at(token, "Expect type.\n");
    }
    
    token = token->next;
    
    return ty;
}

Token* tk_get_token(){
    return token;
}

void tk_set_token(Token* tok){
    token = tok;
}

Token* tk_consume_string(){
    if(token->kind != TK_STRING_CONST){
        return NULL;
    }

    Token* tok = token;
    token = token->next;
    return tok;
}

bool    tk_is_label(){
    bool ret = false;
    Token* tk_bk = token;
    if(tk_consume_ident() && tk_consume(":"))
        ret = true;
    
    token = tk_bk;
    return ret;
}

// local function ---------------------------------
static Token* new_token(TokenKind kind, Token* cur, char* str, int len){
    Token* tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = strndup(str, len);
    tok->len = len;
    tok->src = cur_file;
    tok->row = row;
    tok->pos = str;
    if(kind == TK_NEWLINE){
        row++;
    }
    cur->next = tok;
    return tok;
}

static bool startswith(char* lhs, char* rhs){
    return memcmp(lhs, rhs, strlen(rhs)) == 0;
}

static bool is_ident1(char p){
    return isalpha(p) || p == '_';
}

static bool is_ident2(char p){
    return is_ident1(p) || isdigit(p);
}

static bool is_keyword(char* lhs, char* rhs){

    if(memcmp(lhs, rhs, strlen(rhs)) == 0 && !is_ident2(lhs[strlen(rhs)])){
        return true;
    }
    return false;
}

// Check next token is keyword.
// Then add TK_KEYWORD token, and return true;
static bool check_keyword(char* keyword, char** p, Token** tok){

    if (is_keyword(*p, keyword)){
        int len = strlen(keyword);
        *tok = new_token(TK_KEYWORD, *tok, *p, len);
        *p += len;
        return true;
    }

    return false;
}

static bool check_preprocess(char* directive, char** p, Token** tok){

    if (is_keyword(*p, directive)){
        int len = strlen(directive);
        *tok = new_token(TK_PREPROCESS, *tok, *p, len);
        *p += len;
        return true;
    }

    return false;
}

