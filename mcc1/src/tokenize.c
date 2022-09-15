#include "mcc1.h"
#include "tokenize.h"
#include "type.h"
#include "errormsg.h"
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
static bool check_preprocess(char* directive, char** p, Token** tok, char* start);
static Token* delete_newline_tok(Token* tok);
// -----------------------------------------------
Token* tk_tokenize_file(char* path){

    SrcFile* srcfile = calloc(1, sizeof(SrcFile));
    srcfile->input_data = read_file(path);
    srcfile->path = strdup(path);

    cur_file = srcfile;

    Token* tok = tk_tokenize(srcfile->input_data);
    tok = delete_newline_tok(tok);

    return tok;
}

Token* tk_tokenize(char* p){

    Token head;
    head.next = NULL;
    Token* cur = &head;
    row = 1;

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

        // skip \ \r \n or \ \n
        if(startswith(p, "\\\r\n")){
            p += 3;
            continue;
        }

        if(startswith(p, "\\\n")){
            p += 2;
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

        if(strchr("+-*/,()<>:;={}&|^[].?!%", *p)){
            cur = new_token(TK_OPERAND, cur, p++, 1);
            continue;
        }

        // tokenize preprocessor directive
        if(*p == '#'){
            
            char* start = p++;

            // skip blank
            while(*p == ' ' || *p == '\t') p++;

            if(check_preprocess("include", &p, &cur, start)
                || check_preprocess("if", &p, &cur, start)
                || check_preprocess("ifdef", &p, &cur, start)
                || check_preprocess("ifndef", &p, &cur, start)
                || check_preprocess("else", &p, &cur, start)
                || check_preprocess("endif", &p, &cur, start)
                || check_preprocess("error", &p, &cur, start)
                || check_preprocess("define", &p, &cur, start)
                || check_preprocess("undef", &p, &cur, start)){
                continue;
            }
        }

        if(startswith(p, "defined")){
            int len = strlen("defined");
            cur = new_token(TK_PREPROCESS, cur, p, len);
            p += len;
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
            || check_keyword("signed", &p, &cur)
            || check_keyword("unsigned", &p, &cur)
            || check_keyword("int", &p, &cur)
            || check_keyword("char", &p, &cur)
            || check_keyword("long", &p, &cur)
            || check_keyword("short", &p, &cur)
            || check_keyword("_Bool", &p, &cur)
            || check_keyword("void", &p, &cur)
            || check_keyword("static", &p, &cur)
            || check_keyword("typedef", &p, &cur)
            || check_keyword("enum", &p, &cur)
            || check_keyword("const", &p, &cur)
            || check_keyword("restrict", &p, &cur)
            || check_keyword("volatile", &p, &cur)
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
            cur->val = strtoul(p, &p, 10);
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

unsigned long tk_expect_num(){
    if(token->kind != TK_NUM){
        error_at(token, "expect number.\n");
    }
    unsigned long ret = token->val;
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

    if(ty
        || startswith(token->str, "signed")
        || startswith(token->str, "unsigned")
        || startswith(token->str, "void")
        || startswith(token->str, "char")
        || startswith(token->str, "short")
        || startswith(token->str, "int")
        || startswith(token->str, "long")
        || startswith(token->str, "_Bool")
        || startswith(token->str, "static")
        || startswith(token->str, "const")
        || startswith(token->str, "volatile")
        || startswith(token->str, "signed")
        || startswith(token->str, "unsigned")
        || startswith(token->str, "typedef")
        || startswith(token->str, "enum")
        || startswith(token->str, "struct")){
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

Type* tk_consume_user_type(){
    Type* ty = ty_find_user_type(token->str, token->len);
    if(ty){
        token = token->next;
    }
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

static bool check_preprocess(char* directive, char** p, Token** tok, char* start){

    if (is_keyword(*p, directive)){
        Token* tcur = calloc(1, sizeof(Token));
        tcur->kind = TK_PREPROCESS;
        tcur->len = strlen(directive) + 1; // # + directive 
        tcur->pos = *p;
        tcur->row = row;
        tcur->src = cur_file;
        tcur->str = calloc(1, tcur->len + 1);
        sprintf(tcur->str, "#%s", directive);
        tcur->val = 0;

        *p += tcur->len - 1;
        (*tok)->next = tcur;
        *tok = tcur;
        return true;
    }

    return false;
}

static Token* delete_newline_tok(Token* tok){
    Token head = { 0 };
    Token* cur = &head;
    head.next = tok;

    while(cur->kind != TK_EOF){
        Token* target = cur->next;
        if(target->kind == TK_NEWLINE){
            cur->next = target->next;
            continue;
        }
        cur = cur->next;
    }
    return head.next;
}
