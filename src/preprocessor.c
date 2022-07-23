#include "preprocessor.h"
#include "tokenize.h"
#include "file.h"
#include "errormsg.h"
#include "utility.h"


static Token* del_newline(Token* tok);
static bool equal_token(char* directive, Token* tok);
static Token* read_include(char* path);
static Token* read_stdlib_include(char* path);
static Token* get_end_token(Token* inc);
static bool equal_token(char* directive, Token* tok);
static void add_macro(Token* tok);
static void del_macro(Token* tok);
static Token* make_copy_token(Token* src);
static Macro* find_macro(Token* tok, Macro* mac);
static Token* analyze_ifdef(Token* tok, Token** tail, bool is_ifdef);
static Token* find_newline(Token* tok);
static char* get_header_path(Token* tok);
static char* make_errormsg(Token* tok);
static bool is_funclike(Token* tok);
static Token* read_token_to_eol(Token* tok);
static Token* add_macro_funclike(Token* tok);
static Token* add_macro_objlike(Token* tok);
static Token* replace_token(Token* target, Macro* mac, Macro* list);
static Macro* copy_macro(Macro* mac);
static Token* copy_token_list(Token* tok);
static bool is_expanded(Token* tok, Macro* list);
static Macro* make_param_list(Token* target, Macro* mac, Token** to_tok);
static Token* expand_funclike_macro(Token* target, Macro* mac, Token** to_tok);
static bool is_ifgroup(Token* tok);
static Token* read_if_section(Token* tok);
static long constant_expr(Token* tok);
static void pp_set_token(Token* tok);
static void pp_expect(char* op);
static int pp_expect_num();
static bool pp_consume(char* op);
static bool pp_consume_preprocess(char* prepro);
static int pp_primary();
static int pp_unary();
static int pp_add();
static int pp_mul();
static int pp_bitShift();
static int pp_relational();
static int pp_equality();
static int pp_bitAnd();
static int pp_bitXor();
static int pp_bitOr();
static int pp_logicAnd();
static int pp_logicOr();
static int pp_cond_expr();
static int pp_expr();


static Token* pp_tok = NULL;


static Macro* macro;

Token* preprocess(Token* tok){

    Token head;
    Token* cur = &head;
    head.next = tok;

    do{
        Token* target = cur->next;

        if(target->kind == TK_PREPROCESS){
            // preprocessor directive
            if(equal_token("#include", target)){

                char* inc_path = get_header_path(target->next);

                Token* inc = NULL;
                if(!equal_token("<", target->next)){
                    inc = read_include(inc_path);
                }

                if(inc == NULL){
                    inc = read_stdlib_include(inc_path);
                    if(inc == NULL){
                        error_at(target->next, "Can not find header file.\n");
                    }
                }

                cur->next = inc;
                Token* tail = get_end_token(inc);
                Token* new_line = find_newline(target);
                tail->next = new_line->next;
                cur = tail;
                continue;
            } else if(equal_token("#define", target)){
                add_macro(target->next);
                cur->next = find_newline(target)->next;
                continue;
            } else if(equal_token("#undef", target)){
                del_macro(target->next);
                cur->next = find_newline(target)->next;
                continue;
            } else if(is_ifgroup(target)){
                cur->next = read_if_section(target);
                continue;
            } else if(equal_token("#error", target)){
                char* msg = make_errormsg(target->next);
                error(msg);
                continue;
            }
        } else {
            Macro* mac = find_macro(target, macro);
            if(mac){
                cur->next = replace_token(target, mac, NULL);
            }
        }

        cur = cur->next;

    } while(cur->kind != TK_EOF);

    tok = head.next;

    tok = del_newline(tok);

    return tok;
}

void init_preprocess(){

    Token tok,tok2;
    tok.kind = TK_IDENT;
    tok.str = "MCC_COMPILER";
    tok.len = strlen(tok.str);

    tok2.kind = TK_NEWLINE;

    Macro* mac = calloc(1, sizeof(Macro));
    mac->def = &tok;
    mac->val = &tok2;
    macro = mac;
}

// local function -------------------------------------------------
static Token* del_newline(Token* tok){

    Token* bef = NULL;
    for(Token* cur = tok; cur->kind != TK_EOF; cur = cur->next){
        if(cur->kind == TK_NEWLINE){
            if(bef){
                bef->next = cur->next;
            } else {
                tok = cur->next;
            }
        } else {
            bef = cur;
        }
    }

    return tok;
}

static bool equal_token(char* directive, Token* tok){

    if(tok->len == strlen(directive)
         && memcmp(tok->str, directive, tok->len) == 0){
        return true;
    }

    return false;
}

static Token* read_include(char* path){

    Token* inc = NULL;
    char* inc_path = get_include_path(path);

    if(inc_path){
        inc = tk_tokenize_file(inc_path);
    }

    return inc;
}

static Token* read_stdlib_include(char* path){

    char* inc_path = calloc(1, strlen(path) + strlen(STDLIB_PATH));
    strcat(inc_path, STDLIB_PATH);
    strcat(inc_path, path);
    Token* inc = tk_tokenize_file(inc_path);

    return inc;
}

static Token* get_end_token(Token* inc){
    
    Token* bef;

    while(inc->kind != TK_EOF){
        bef = inc;
        inc = inc->next;
    }

    return bef;
}

static Macro* copy_macro(Macro* mac){
    Macro* ret = calloc(1, sizeof(Macro));
    memcpy(ret, mac, sizeof(Macro));
    ret->next = NULL;
    
    return ret;
}

static Token* copy_token_list(Token* tok){
    Token head;
    Token* cur = &head;
    while(tok){
        cur->next = make_copy_token(tok);
        cur = cur->next;
        tok = tok->next;
    }

    return head.next;
}

static bool is_expanded(Token* tok, Macro* list){

    if(tok->kind != TK_IDENT){
        return false;
    }

    Macro* cur = list;
    while(cur){
        if(equal_token(tok->str, cur->def)){
            return true;
        }
    }

    return false;
}

static Macro* make_param_list(Token* target, Macro* mac, Token** to_tok){

    Macro head;
    Macro* arg_list = &head;
    target = target->next;  // skip functype macro ident
    target = target->next;  // skip lparam
    
    Token* param = mac->param;

    while(!equal_token(")", target)){

        arg_list = arg_list->next = calloc(1, sizeof(Macro));
        
        arg_list->def = make_copy_token(param);
        arg_list->val = make_copy_token(target);
        if(arg_list->val->kind == TK_NUM){
            arg_list->val->str = strndup(arg_list->val->str, arg_list->val->len);
        }

        param = param->next;
        
        target = target->next;
        if(equal_token(")", target)){
            break;
        }

        if(!equal_token(",", target)){
            error_at(target, "expect , operater.\n");
        }
        target = target->next;
    }

    *to_tok = target->next;

    return head.next;
}

static Token* expand_funclike_macro(Token* target, Macro* mac, Token** to_tok){

    // make param list
    Macro* param_list = make_param_list(target, mac, to_tok);

    // change parameter to arguments
    Token head;
    head.next = mac->val;
    Token* cur = &head;
    while(cur->next){
        Token* tok = cur->next;

        Macro* m = find_macro(tok, param_list);
        if(m){
            cur->next = make_copy_token(m->val);
            cur->next->next = tok->next;
        }
        cur = cur->next;
    }

    return head.next;
}

static Token* replace_token(Token* target, Macro* mac, Macro* list){

    // add expand list.
    if(!list){
        list = copy_macro(mac);
    } else {
        list->next = copy_macro(mac);
    }

    // get macro value.
    Token* val = NULL;
    Token* to_tok = NULL;
    if(mac->is_func){
        val = expand_funclike_macro(target, mac, &to_tok);
    } else {
        val = copy_token_list(mac->val);
        to_tok = target->next;
    }

    // expand macro to macro value. 
    Token head;
    head.next = val;
    Token* cur = &head;
    while(cur->next){
        Token* target = cur->next;

        // skip expanded macro.
        if(is_expanded(target, list)){
            cur = cur->next;
            continue;
        }

        Macro* m = find_macro(target, macro);
        if(m){
            cur->next = replace_token(target, m, list);
        }
        cur = cur->next;
    }

    // connect token
    cur = val;
    while(cur->next){
        cur = cur->next;
    }

    cur->next = to_tok;

    return val;
}

static bool is_funclike(Token* tok){
    return *(tok->pos + tok->len) == '(';
}

static Token* read_token_to_eol(Token* tok){
    Token head;
    Token* cur = &head;
    Token* c_tok = tok;
    while(c_tok->kind != TK_NEWLINE){
        cur->next = make_copy_token(c_tok);
        cur = cur->next;
        c_tok = c_tok->next;
    }

    cur->next = make_copy_token(c_tok);
    cur = cur->next;

    return head.next;
}

static Token* add_macro_funclike(Token* tok){
    if(find_macro(tok, macro)){
        error_at(tok, "Redefined macro.\n");
    }

    Macro* mac = calloc(1, sizeof(Macro));

    mac->def = make_copy_token(tok);
    tok = tok->next;    // progress to '(' token.
    tok = tok->next;    // progress to first parameter token.

    // read paramater.
    Token head;
    Token* cur = &head;
    while(!equal_token(")", tok)){
        cur->next = make_copy_token(tok);
        cur = cur->next;
        tok = tok->next;

        if(equal_token(",", tok)){
            tok = tok->next;
        }
    }
    tok = tok->next;

    mac->param = head.next;
    mac->val = read_token_to_eol(tok);
    mac->is_func = true;

    mac->next = macro;
    macro = mac;
}

static Token* add_macro_objlike(Token* tok){
    if(find_macro(tok, macro)){
        error_at(tok, "Redefined macro.\n");
    }

    Macro* mac = calloc(1, sizeof(Macro));

    mac->def = make_copy_token(tok);
    mac->val = read_token_to_eol(tok->next);

    mac->next = macro;
    macro = mac;
}

static void add_macro(Token* tok){

    if(tok->kind != TK_IDENT){
        error_at(tok, "Expect identify token.");
    }

    if(is_funclike(tok)){
        add_macro_funclike(tok);
    } else {
        add_macro_objlike(tok);
    }
}

static void del_macro(Token* tok){

    Macro head;
    head.next = macro;
    Macro* cur = &head;
    while(cur){
        Macro* target = cur->next;
        if(equal_token(tok->str, target->def)){
            cur->next = target->next;
            free(target);
            break;
        }
        cur = cur->next;
    }
}

static Token* make_copy_token(Token* src){
    Token* new_tok = calloc(1, sizeof(Token));
    memcpy(new_tok, src, sizeof(Token));
    new_tok->next = NULL;

    return new_tok;
}

static Macro* find_macro(Token* tok, Macro* mac){

    if(tok->kind != TK_IDENT){
        return NULL;
    }

    for(Macro* cur = mac; cur; cur = cur->next){
        if(equal_token(tok->str, cur->def)){
            return cur;
        }
    }

    return NULL;
}

static bool is_ifgroup(Token* tok){
    if(equal_token("#if", tok)
    || equal_token("#ifdef", tok)
    || equal_token("#ifndef", tok))
    {
        return true;
    }
    return false;
}

static bool is_branch(Token* tok){
    if(equal_token("#elif", tok)
    || equal_token("#else", tok)
    || equal_token("#endif", tok))
    {
        return true;
    }
    return false;
}

static Token* get_nl_token(Token* tok){
    while(tok){
        if(tok->kind == TK_NEWLINE){
            return tok;
        }

        tok = tok->next;
    }
    return NULL;
}

static bool read_cond(Token* tok){

    if(equal_token("#if", tok)){
        return constant_expr(read_token_to_eol(tok->next)) != 0;
    } else if(equal_token("#elif", tok)){
        return constant_expr(read_token_to_eol(tok->next)) != 0;
    } else if(equal_token("#ifdef", tok)){
        return find_macro(tok->next, macro) != NULL;
    } else if(equal_token("#ifndef", tok)){
        return find_macro(tok->next, macro) == NULL;
    } else if(equal_token("#else", tok)){
        return true;
    }
}

static Token* get_endif(Token* tok){

    Token* cur = tok->next;
    while(cur){
        if(equal_token("#endif", cur)){
            return cur;
        } else if(is_ifgroup(cur)){
            cur = get_endif(cur);
        }

        cur = cur->next;
    }

}

static Token* read_if_section(Token* tok){

    // ans token
    Token* h = get_nl_token(tok)->next;
    Token* rt = h;
    Token* rh = NULL;
    bool cur_cond = read_cond(tok);

    Token head;
    Token* cur = &head;
    head.next = get_nl_token(tok)->next;
    while(cur->next){
        Token* target = cur->next;

        if(equal_token("#endif", target)){
            if(cur_cond && !rh){
                rh = h;
                rt = cur;
            }
            break;
        } else if(equal_token("#else", target)){
            if(cur_cond && !rh){
                rh = h;
                rt = cur;
            }
            cur_cond = true;
            h = target->next;
        } else if(is_ifgroup(target)){
            cur = get_endif(target);
        }

        cur = cur->next;
    }

    if(!rh){
        rh = cur->next->next;
    } else {
        rt->next = cur->next->next;
    }

    return rh;
}

static Token* analyze_ifdef(Token* tok, Token** tail, bool is_ifdef){

    bool is_defined = is_ifdef == (find_macro(tok, macro) != NULL);
    Token* t_head = tok->next;
    Token* t_tail = NULL;
    Token* f_head = NULL;
    Token* f_tail = NULL;
    bool tf_path = true;

    Token head;
    Token* cur = &head;
    head.next = tok->next;

    while(cur){
        Token* target = cur->next;

        if(equal_token("#endif", target)){
            cur->next = target->next;
            if(tf_path){
                t_tail = cur;
            } else {
                f_tail = cur;
                t_tail->next = cur->next;
            }
            break; 
        } else if(equal_token("#else", target)){
            tf_path = false;
            t_tail = cur;
            f_head = target->next;
        } else if(equal_token("#ifdef", target)){
            Token* ifdef_tail;
            Token* ifdef_head = analyze_ifdef(target->next, &ifdef_tail, true);

            cur->next = ifdef_head;
            cur = ifdef_tail;
        } else if(equal_token("#ifndef", target)){
            Token* ifdef_tail;
            Token* ifdef_head = analyze_ifdef(target->next, &ifdef_tail, false);

            cur->next = ifdef_head;
            cur = ifdef_tail;
        }
        cur = cur->next;
    }

    if(is_defined){
        if(t_tail)
            *tail = t_tail;

        else {
            *tail = cur->next;
            t_head = *tail;
        }
        return t_head;
    } else {
        if(f_tail != NULL){
            *tail = f_tail;
            return f_head;
        } else {
            *tail = t_tail;
            return t_tail;
        }
    }
}

static Token* find_newline(Token* tok){
    Token* cur = tok;

    while(cur->kind != TK_NEWLINE){
        cur = cur->next;
        if(cur->next->kind == TK_EOF){
            error_at(cur, "Reach eof before find new-line token.\n");
        }
    }
    return cur;
}

static char* get_header_path(Token* tok){

    if(tok->kind == TK_STRING_CONST){
        return strndup(tok->str, tok->len);
    }

    if(tok->kind != TK_OPERAND || !equal_token("<", tok)){
        error_at(tok, "Expect include file path.\n");
    }

    int len = 0;
    Token* cur = tok->next;
    while(!equal_token(">", cur)){
        len += cur->len;
        cur = cur->next;
    }

    char* inc_path = calloc(1, len);
    cur = tok->next;
    while(!equal_token(">", cur)){
        strcat(inc_path, cur->str);
        cur = cur->next;
    }

    return inc_path;
}

static char* make_errormsg(Token* tok){

    int len = 0;
    char* p = tok->pos;
    while(*p != '\n'){
        len++;
        p++;
    }

    char* msg = calloc(1, len);
    strncpy(msg, tok->pos, len);

    return msg;
}

static void pp_set_token(Token* tok){
    pp_tok = tok;
}

static void pp_expect_newline(){
    if(pp_tok->kind != TK_NEWLINE){
        error_at(pp_tok, "Expect newline, but this is another one.\n");
    }
}

static void pp_expect(char* op){
    if(!pp_consume(op)){
        error_at(pp_tok, "No expected tokens.\n");
    }
}

static int pp_expect_num(){
    if(pp_tok->kind != TK_NUM)
        error_at(pp_tok, "expect num\n");
    
    int ans = pp_tok->val;
    pp_tok = pp_tok->next;
    return ans;
}

static bool pp_consume(char* op){
    if(equal_token(op, pp_tok)
        && pp_tok->kind == TK_OPERAND){
        pp_tok = pp_tok->next;
        return true;
    }
    return false;
}

static bool pp_consume_preprocess(char* prepro){
    if(equal_token(prepro, pp_tok)
        && pp_tok->kind == TK_PREPROCESS){
        pp_tok = pp_tok->next;
        return true;
    }
    return false;
}

static int pp_expr(){
    int ans = pp_cond_expr();
    return ans;
}

static int pp_cond_expr(){
    int ans = pp_logicOr();

    if(pp_consume("?")){
        int ans_tpath = pp_expr();
        pp_expect(":");
        int ans_fpath = pp_expr();

        ans = ans ? ans_tpath : ans_fpath;
    }
    return ans;
}

static int pp_logicOr(){
    int ans = pp_logicAnd();

    for(;;){
        if(pp_consume("||")){
            ans = ans || pp_logicAnd();
        } else {
            return ans;
        }
    }
}

static int pp_logicAnd(){
    int ans = pp_bitOr();

    for(;;){
        if(pp_consume("&&")){
            ans = ans && pp_bitOr();
        } else {
            return ans;
        }
    }
}

static int pp_bitOr(){
    int ans = pp_bitXor();

    for(;;){
        if(pp_consume("|")){
            ans |= pp_bitXor();
        } else {
            return ans;
        }
    }
}

static int pp_bitXor(){
    int ans = pp_bitAnd();

    for(;;){
        if(pp_consume("^")){
            ans ^= pp_bitAnd();
        } else {
            return ans;
        }
    }
}

static int pp_bitAnd(){
    int ans = pp_equality();

    for(;;){
        if(pp_consume("&")){
            ans &= pp_equality();
        } else {
            return ans;
        }
    }
}

static int pp_equality(){
    int ans = pp_relational();

    for(;;){
        if(pp_consume("==")){
            ans = ans == pp_relational();
        } else if(pp_consume("!=")){
            ans = ans != pp_relational();
        } else {
            return ans;
        }
    }
}

static int pp_relational(){
    int ans = pp_bitShift();

    for(;;){
        if(pp_consume("<")){
            ans = ans < pp_bitShift();
        } else if(pp_consume("<=")){
            ans = ans <= pp_bitShift();
        } else if(pp_consume(">")){
            ans = ans > pp_bitShift();
        } else if(pp_consume(">=")){
            ans = ans >= pp_bitShift();
        } else {
            return ans;
        }
    }
}

static int pp_bitShift(){
    int ans = pp_add();
    for(;;){
        if(pp_consume("<<")){
            ans <<= pp_add();
        } else if(pp_consume(">>")){
            ans >>= pp_add();
        } else {
            return ans;
        }
    }
}

static int pp_add(){
    int ans = pp_mul();

    for(;;){
        if(pp_consume("+")){
            ans += pp_mul();
        } else if(pp_consume("-")){
            ans -= pp_mul();
        } else {
            return ans;
        }
    }
}

static int pp_mul(){
    int ans = pp_unary();
    for(;;){
        if(pp_consume("*")){
            ans *= pp_unary();
        } else if(pp_consume("/")){
            ans /= pp_unary();
        } else if(pp_consume("%")){
            ans %= pp_unary();
        } else {
            return ans;
        }
    }
}

static int pp_unary(){
    if(pp_consume("+")){
        return pp_primary();
    } else if(pp_consume("-")){
        return -pp_primary();
    } else if(pp_consume_preprocess("defined")){
        int ans = 0;
        Token* tok;
        if(pp_consume("(")){
            tok = pp_tok;
            pp_tok = pp_tok->next;
            pp_expect(")");
        } else {
            tok = pp_tok;
            pp_tok = pp_tok->next;
        }
        if(find_macro(tok, macro)){
            ans = 1;
        }
        return ans;
    } else {
        return pp_primary();
    }
}

static int pp_primary(){

    if(pp_consume("(")){
        int ans = pp_expr();
        pp_expect(")");
        return ans;
    } else {
        return pp_expect_num();
    }
}

static Token* pp_expand_defined(Token* tok){

    Token head;
    Token* cur = &head;
    head.next = tok;
    while(cur->next){
        Token* target = cur->next;
        if(equal_token("defined", target)){
            Token* ident;
            target = target->next;

            if(equal_token("(", target)){
                target = target->next;  // ( -> ident
                ident = target;
                
                target = target->next;  // ident -> )
                if(!equal_token(")", target)){
                    error_at(tok, "Expect ')' token, but get another one.\n");
                }
            } else {
                ident = target;
            }

            target = target->next; // ')' or ident -> next;

            Token* val = calloc(1, sizeof(Token));
            val->kind = TK_NUM;
            val->val = find_macro(ident, macro) ? 1 : 0;

            cur->next = val;
            val->next = target;
            cur = target;
        } else {
            cur = cur->next;
        }
    }
    return head.next;
}

static Token* pp_expand_macros(Token* tok){
    Token head;
    Token* cur = &head;
    head.next = tok;
    while(cur->next){
        Token* target = cur->next;
        Macro* mac = find_macro(target, macro);
        if(mac){
            cur->next = replace_token(target, mac, NULL);
        }
        cur = cur->next;
    }
    return head.next;
}

static long constant_expr(Token* tok){
    tok = pp_expand_defined(tok);
    tok = pp_expand_macros(tok);
    pp_set_token(tok);
    int ans = pp_expr();
    pp_expect_newline();
    return ans;
}