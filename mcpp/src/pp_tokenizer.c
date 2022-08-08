#include "mcpp.h"

PP_Token* token = NULL;
SrcFile* cur_file = NULL;
unsigned long row = 0;

static PP_Token* ptk_tokenize(char* path);
static bool startswith(char* lhs, char* rhs);
static bool check_keyword(char* keyword, char** p, PP_Token** tok, PP_TokenKind kind);
static bool is_keyword(char* lhs, char* rhs);
static bool is_ident1(char p);
static bool is_ident2(char p);

PP_Token* ptk_tokenize_file(char* path){

    SrcFile* srcfile = calloc(1, sizeof(SrcFile));
    srcfile->input_data = read_file(path);
    srcfile->path = strdup(path);

    free(cur_file);
    cur_file = srcfile;

    PP_Token* tok = ptk_tokenize(cur_file->input_data);

    return tok;
}

static PP_Token* ptk_tokenize(char* p){
    PP_Token head;
    head.next = NULL;
    PP_Token* cur = &head;
    row = 1;

    while(*p){      

        if(startswith(p, "\r\n")){
            cur = new_token(PTK_NEWLINE, cur, p, 2);
            p += 2;
            continue;
        }

        if(*p == '\r' || *p == '\n'){
            cur = new_token(PTK_NEWLINE, cur, p++, 1);
            continue;
        }

        // ----------------------------------
        //      skip \ and new-line.     
        // ----------------------------------
        if(startswith(p, "\\\r\n")){
            p += 3;
            continue;
        }

        if(startswith(p, "\\\n")){
            p += 2;
            continue;
        }

        if(isspace(*p)){
            char* start = p;
            do { 
                p++;
            } while(isspace(*p));
            cur = new_token(PTK_SPACE, cur, start, p - start);
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
            cur = new_token(PTK_STRING_CONST, cur, start, p - start);
            p++;
            continue;
        }

        if(*p == '\''){
            char a = *(++p);
            cur = new_token(PTK_NUM, cur, p, 1);
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
            cur = new_token(PTK_OPERAND, cur, p, 3);
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
            cur = new_token(PTK_OPERAND, cur, p, 2);
            p += 2;
            continue;
        }

        if(strchr("+-*/,()<>:;={}&|^[].?!%", *p)){
            cur = new_token(PTK_OPERAND, cur, p++, 1);
            continue;
        }

        if(*p == '#'){
            cur = new_token(PTK_HASH, cur, p++, 1);
            continue;
        }

        if(check_keyword("include", &p, &cur, PTK_PP_KEYWORD)
            || check_keyword("if", &p, &cur, PTK_PP_KEYWORD)
            || check_keyword("ifdef", &p, &cur, PTK_PP_KEYWORD)
            || check_keyword("ifndef", &p, &cur, PTK_PP_KEYWORD)
            || check_keyword("else", &p, &cur, PTK_PP_KEYWORD)
            || check_keyword("endif", &p, &cur, PTK_PP_KEYWORD)
            || check_keyword("error", &p, &cur, PTK_PP_KEYWORD)
            || check_keyword("define", &p, &cur, PTK_PP_KEYWORD)
            || check_keyword("defined", &p, &cur, PTK_PP_KEYWORD)
            || check_keyword("undef", &p, &cur, PTK_PP_KEYWORD)){
                continue;
        }

        if(is_ident1(*p)){
            char* start = p;
            p++;
            while(is_ident2(*p)){
                p++;
            }

            cur = new_token(PTK_IDENT, cur, start, p - start);
            continue;
        }

        if(isdigit(*p)){
            cur = new_token(PTK_NUM, cur, p, 0);
            char* q = p;
            cur->val = strtol(p, &p, 10);
            cur->len = p - q;
            continue;
        }

        error("find cannnot tokenize words.\n");
    }

    cur = new_token(PTK_EOF, cur, p, 0);

    return head.next;
}

PP_Token* new_token(PP_TokenKind kind, PP_Token* cur, char* str, int len){
    PP_Token* tok = calloc(1, sizeof(PP_Token));
    tok->kind = kind;
    tok->str = strndup(str, len);
    tok->len = len;
    tok->src = cur_file;
    tok->row = row;
    tok->pos = str;
    if(kind == PTK_NEWLINE){
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
static bool check_keyword(char* keyword, char** p, PP_Token** tok, PP_TokenKind kind){

    if (is_keyword(*p, keyword)){
        int len = strlen(keyword);
        *tok = new_token(kind, *tok, *p, len);
        *p += len;
        return true;
    }

    return false;
}
