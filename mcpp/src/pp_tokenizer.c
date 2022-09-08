#include "mcpp.h"

PP_Token* token = NULL;
SrcFile* cur_file = NULL;
unsigned long row = 0;

static bool startswith(char* lhs, char* rhs);
static bool check_keyword(char* keyword, char** p, PP_Token** tok, PP_TokenKind kind);
static bool is_keyword(char* lhs, char* rhs);
static bool is_ident1(char p);
static bool is_ident2(char p);
static PP_Token* read_string_literal(char* p, PP_Token* cur);
static char* get_string_literal_end(char* p);

PP_Token* ptk_tokenize_file(char* path){

    SrcFile* srcfile = calloc(1, sizeof(SrcFile));
    srcfile->input_data = read_file(path);
    srcfile->path = strdup(path);

    cur_file = srcfile;

    PP_Token* tok = ptk_tokenize(cur_file->input_data);

    return tok;
}

PP_Token* ptk_tokenize(char* p){
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
            row++;
            continue;
        }

        if(startswith(p, "\\\n")){
            p += 2;
            row++;
            continue;
        }

        if(isspace(*p) && *p != '\n'){
            char* start = p;
            do { 
                p++;
            } while(isspace(*p) && *p != '\n');
            cur = new_token(PTK_SPACE, cur, start, p - start);
            continue;
        }

        if(strncmp(p, "//", 2) == 0){
            p += 2;
            while(*p != '\n'){
                p++;
            }
            p++;
            cur = new_token(PTK_NEWLINE, cur, p, 1);
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
            cur = read_string_literal(p, cur);
            p += cur->len + 2;
            continue;
        }

        if(*p == 0x27){
            cur = new_token(PTK_CHAR_CONST, cur, p, 1);

            // escape sequence
            if(*(p + 1) == 0x5c){
                p += 2;
                switch(*p){
                    case 'a':               // ring a bell
                        cur->val = 0x07;
                        break;
                    case 'b':               // back space
                        cur->val = 0x08;
                        break;
                    case 'f':               // form feed
                        cur->val = 0x0c;
                        break;                   
                    case 'n':
                        cur->val = 0x0a;    // LF
                        break;
                    case 'r':
                        cur->val = 0xd;     // CR
                        break;
                    case 't':
                        cur->val = 0x09;    // Horizontal Tab
                        break;
                    case 'v':               // vertical Tab
                        cur->val = 0x0b;
                        break;
                    case '\'':              // single quote
                        cur->val = 0x27;
                        p++;
                        break;
                    case '"':               // double quote
                        cur->val = 0x22;
                        break;
                    case '\\':              // back slash
                        cur->val = 0x5c;
                        break;
                    case '0':               // null caracter
                        cur->val = 0x00;
                        break;
                }
            } else {
                cur->val = *(++p);
            }

            while(*p != 0x27){
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

        if(strchr("+-*/,()<>:;={}&|^[].?!%~", *p)){
            cur = new_token(PTK_OPERAND, cur, p++, 1);
            continue;
        }

        if(startswith(p, "##")){
            cur = new_token(PTK_HASH_HASH, cur, p ,2);
            p += 2;
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

        if(*p == '0') {
                
            if((*(p + 1) == 'x')
                || (*(p + 1) == 'X')){
                cur = new_token(PTK_NUM, cur, p, 0);
                char* q = p;
                cur->val = strtol(p, &p, 16);
                cur->len = p - q;
                cur->is_hex = true;
                cur->str = strndup(q, cur->len);
                continue;
            } else if(isdigit(*(p + 1))){
                cur = new_token(PTK_NUM, cur, p, 0);
                char* q = p;
                cur->val = strtol(p, &p, 8);
                cur->len = p - q;
                cur->str = strndup(q, cur->len);
                continue;
            }
        }

        if(isdigit(*p)){
            cur = new_token(PTK_NUM, cur, p, 0);
            char* q = p;
            cur->val = strtol(p, &p, 10);
            if(*p == 'L') p++;
            cur->len = p - q;
            cur->str = strndup(q, cur->len);
            continue;
        }

        if(cur_file){
            fprintf(stderr, "filepath : %s, line : %ld\n", cur_file->path, row);
        }
        error("find cannnot tokenize words.\n");
    }

    cur = new_token(PTK_NEWLINE, cur, p, 0);
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

static PP_Token* read_string_literal(char* p, PP_Token* cur){
    char* start = p + 1;
    char* end = get_string_literal_end(p + 1);

    PP_Token* ret = new_token(PTK_STRING_CONST, cur, start, end - start);
    return ret;
}

// find end of double-quote
static char* get_string_literal_end(char* p){
    char* start = p;
    for(; *p != '"'; p++){
        if(*p == '\\') {
            // escape sequence skip next character.
            p++;
        }
    }
    return p;
}