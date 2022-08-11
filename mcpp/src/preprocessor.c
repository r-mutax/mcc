#include "mcpp.h"

static Macro* macro;

// if-group section
static PP_Token* read_if_section(PP_Token* tok);
static bool analyze_if_condition(PP_Token* hash);
static int evaluate_expr(PP_Token* tok);
static PP_Token* expand_defined(PP_Token* tok);


// macro definition
static void add_macro(PP_Token* tok);
static bool is_funclike_macro(PP_Token* tok);
static void add_macro_objlike(PP_Token* tok);
static Macro* find_macro(PP_Token* tok, Macro* mac);
static PP_Token* replace_token(PP_Token* target, Macro* mac, Macro* list);
static bool is_expanded(PP_Token* tok, Macro* list);

// undef ddirective
static void del_macro(PP_Token* tok);

// error directive
static char* make_errormsg(PP_Token* target);

// utilityies
static PP_KIND get_preprocess_kind(PP_Token* token);
static bool equal_token(const char* word, PP_Token* target);
static PP_Token* get_next_newline(PP_Token* tok);
static Macro* copy_macro(Macro* mac);
static PP_Token* copy_token_list(PP_Token* tok);
static PP_Token* copy_token(PP_Token* tok);
static PP_Token* copy_token_eol(PP_Token* tok);
static PP_Token* get_directive_value(PP_Token* hash);
static PP_Token* get_endif(PP_Token* tok);
static PP_Token* get_next(PP_Token* tok);

// preprocess exchange 
PP_Token* preprocess(PP_Token* tok){

    PP_Token head = { 0 };
    PP_Token* cur = &head;
    head.next = tok;

    do {
        PP_Token* target = cur->next;

        if(target->kind == PTK_HASH){
            
            switch(get_preprocess_kind(target)){
                case PP_INCLUDE:
                    break;
                case PP_IF:
                case PP_IFDEF:
                case PP_IFNDEF:
                    cur->next = read_if_section(target);
                    continue;
                case PP_ERROR:
                {
                    char* msg = make_errormsg(target);
                    error(msg);
                    continue;
                }
                case PP_DEFINE:
                    add_macro(target);
                    cur->next = get_next_newline(target);
                    continue;
                case PP_UNDEF:
                    del_macro(target);
                    cur->next = get_next_newline(target);
                    continue;
                default:
                    error("unexpected preprocessor directive.\n");
                    break;
            }
        } else {
            // replace token
            Macro* mac = find_macro(target, macro);
            if(mac){
                cur->next = replace_token(target, mac, NULL);
            }
        }

        cur = cur->next;
    } while(cur->kind != PTK_EOF);

    return head.next;
}

static bool equal_token(const char* word, PP_Token* target){
    if(target->len == strlen(word)
        && memcmp(target->str, word, target->len) == 0){
        return true;
    }

    return false;
}

static PP_KIND get_preprocess_kind(PP_Token* token){

    PP_Token* target = token->next;
    if(target->kind == PTK_SPACE){
        target = target->next;
    }

    PP_KIND kind = PP_NONE;
    if(equal_token("include", target)){
        kind = PP_INCLUDE;
    } else if(equal_token("if", target)){
        kind = PP_IF;
    } else if(equal_token("ifdef", target)){
        kind = PP_IFDEF;
    } else if(equal_token("ifndef", target)){
        kind = PP_IFNDEF;
    } else if(equal_token("elif", target)){
        kind = PP_ELIF;
    } else if(equal_token("else", target)){
        kind = PP_ELSE;
    } else if(equal_token("endif", target)){
        kind = PP_ENDIF;
    } else if(equal_token("error", target)){
        kind = PP_ERROR;
    } else if(equal_token("define", target)){
        kind = PP_DEFINE;
    } else if(equal_token("defined", target)){
        kind = PP_DEFINED;
    } else if(equal_token("undef", target)){
        kind = PP_UNDEF;
    }

    return kind;
}

static Macro* find_macro(PP_Token* tok, Macro* mac){

    if(tok->kind != PTK_IDENT){
        return NULL;
    }

    for(Macro* cur = mac; cur; cur = cur->next){
        if(equal_token(tok->str, cur->def)){
            return cur;
        }
    }
}

static void add_macro(PP_Token* tok){

    tok = get_directive_value(tok);

    if(tok->kind != PTK_IDENT){
        error_at(tok, "[error] Expect identity token.");
    }

    if(is_funclike_macro(tok)){
        
    } else {
        add_macro_objlike(tok);
    }
}

static bool is_funclike_macro(PP_Token* tok){
    return equal_token("(", tok->next);
}

static void add_macro_objlike(PP_Token* tok){
    if(find_macro(tok, macro)){
        error_at(tok, "[error] redefined macro.\n");
    }

    Macro* mac = calloc(1, sizeof(Macro));

    mac->def = copy_token(tok);
    tok = tok->next->next;
    mac->val = copy_token_eol(tok);

    mac->next = macro;
    macro = mac;
}

static PP_Token* replace_token(PP_Token* tok, Macro* mac, Macro* list){

    // add expand list
    if(!list){
        list = copy_macro(mac);
    } else {
        list->next = copy_macro(mac);
    }

    // get macro value.
    PP_Token* val = NULL;
    PP_Token* to_tok = NULL;
    if(mac->is_func){
        // TODO : expand funclike macro.
    } else {
        val = copy_token_list(mac->val);
        to_tok = tok->next;
    }

    // expand macro to macro value.
    PP_Token head;
    head.next = val;
    PP_Token* cur = &head;
    while(cur->next){
        PP_Token* target = cur->next;

        if(is_expanded(target, list)){
            // "target" is expanded macro, skip expanding macro.
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

static bool is_expanded(PP_Token* tok, Macro* list){

    if(tok->kind != PTK_IDENT){
        return false;
    }

    Macro* cur = list;
    while(cur){
        if(equal_token(tok->str, cur->def)){
            return true;
        }
        cur = cur->next;
    }

    return false;
}

static PP_Token* get_next_newline(PP_Token* tok){
    PP_Token* start = tok;
    while(tok->kind != PTK_NEWLINE){
        if(tok->kind == PTK_EOF){
            error_at(start, "[error] reach end of file.\n");
        }
        tok = tok->next;
    }
    return tok;
}

static Macro* copy_macro(Macro* mac){
    Macro* ret = calloc(1, sizeof(Macro));
    memcpy(ret, mac, sizeof(Macro));
    ret->next = NULL;
    
    return ret;
}

static void del_macro(PP_Token* tok){

    tok = get_directive_value(tok);

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
    macro = head.next;
}

static PP_Token* copy_token_list(PP_Token* tok){
    PP_Token head;
    PP_Token* cur = &head;
    while(tok){
        cur->next = copy_token(tok);
        cur = cur->next;
        tok = tok->next;
    }

    return head.next;
}

static PP_Token* copy_token(PP_Token* src){
    PP_Token* new_tok = calloc(1, sizeof(PP_Token));
    memcpy(new_tok, src, sizeof(PP_Token));
    new_tok->next = NULL;

    return new_tok;
}

static PP_Token* copy_token_eol(PP_Token* tok){
    PP_Token head;
    PP_Token* cur = &head;
    PP_Token* c_tok = tok;
    while(c_tok->kind != PTK_NEWLINE){
        cur->next = copy_token(c_tok);
        cur = cur->next;
        c_tok = c_tok->next;
    }

    return head.next;
}

static char* make_errormsg(PP_Token* target){

    target = get_directive_value(target);

    int len = 0;
    char* start = target->pos;
    while(target){
        if (target->kind == PTK_NEWLINE){
            break;
        }
        len += target->len;
        target = target->next;
    }

    char* msg = calloc(1, len);
    strncpy(msg, start, len);

    return msg;
}

static PP_Token* get_directive_value(PP_Token* hash){

    PP_Token* target = hash;

    target = target->next;  // hash -> directive
    target = target->kind == PTK_SPACE ? target->next : target;
    target = target->next;  // directive -> space
    target = target->next;

    return target;
}

static PP_Token* read_if_section(PP_Token* tok){

    IF_SECTION_GROUP if_head;
    IF_SECTION_GROUP* if_group = &if_head;
    if_group = if_group->next = calloc(1, sizeof(IF_SECTION_GROUP));
    if_group->cond = analyze_if_condition(tok);
    if_group->head = get_next_newline(tok)->next;

    PP_Token head;
    head.next = if_group->head;
    PP_Token* cur = &head;
    PP_Token* endpos = NULL;
    while(cur){
        PP_Token* target = cur->next;
        bool exit_flg = false;

        if(target->kind == PTK_HASH){
            switch(get_preprocess_kind(target)){
                case PP_IF:
                case PP_IFDEF:
                case PP_IFNDEF:
                    cur = get_endif(get_next_newline(target));
                    cur = get_next_newline(cur);
                    continue;
                case PP_ELIF:
                    if_group->tail = cur;
                    if_group = if_group->next = calloc(1, sizeof(IF_SECTION_GROUP));
                    if_group->cond = analyze_if_condition(target);
                    if_group->head = get_next_newline(target)->next;
                    break;
                case PP_ELSE:
                    if_group->tail = cur;

                    if_group = if_group->next = calloc(1, sizeof(IF_SECTION_GROUP));
                    if_group->cond = 1;
                    if_group->head = get_next_newline(target)->next;
                    break;
                case PP_ENDIF:
                    if_group->tail = cur;
                    exit_flg = true;
                    endpos = target;
                    break;
            }
        }

        if(exit_flg){
            break;
        }
        cur = cur->next;
    }

    IF_SECTION_GROUP* ret = NULL;
    for(IF_SECTION_GROUP* cur = if_head.next; cur; cur = cur->next){
        if(cur->cond){
            ret = cur;
            ret->tail->next = get_next_newline(endpos);
            return ret->head;
        }
    }
    
    return get_next_newline(endpos);
}

static PP_Token* get_endif(PP_Token* tok){

    PP_Token head;
    PP_Token* cur = &head;
    head.next = tok->next;

    while(cur){
        PP_Token* target = cur->next;
        if(target->kind == PTK_HASH){
            switch(get_preprocess_kind(target)){
                case PP_ENDIF:
                    return target;
                case PP_IF:
                case PP_IFDEF:
                case PP_IFNDEF:
                    cur = get_endif(cur);
                    break;
            }
        } else if(target->kind == PTK_EOF) {
            error_at(tok, "[error] reach EOF before find #endif.\n");
        }

        cur = cur->next;
    }
}

static PP_Token* get_next(PP_Token* tok){
    tok = tok->next;
    if(tok->kind == PTK_SPACE){
        tok = tok->next;
    }
    return tok;
}

static bool analyze_if_condition(PP_Token* hash){

    PP_Token* val = get_directive_value(hash);
    switch(get_preprocess_kind(hash)){
        case PP_IF:
            return evaluate_expr(val);
        case PP_IFDEF:
            if(find_macro(val, macro)){
                return 1;
            }
        case PP_IFNDEF:
            if(!find_macro(val, macro)){
                return 1;
            }
        case PP_ELIF:
            return evaluate_expr(val);
    }
    return 0;
}

static int evaluate_expr(PP_Token* tok){
    tok = expand_defined(tok);
    return constant_expr(tok);
}

static PP_Token* expand_defined(PP_Token* tok){
    PP_Token head;
    PP_Token* cur = &head;
    head.next = tok;
    while(get_next(cur)->kind != PTK_NEWLINE){
        PP_Token* target = get_next(cur);
        
        if(equal_token("defined", target)){
            PP_Token* ident;
            target = get_next(target);

            if(equal_token("(", target)){
                ident = target = get_next(target);
                target = get_next(target);
                if(!equal_token(")", target)){
                    error_at(tok, "Expect ')' token.\n");
                }
            } else {
                ident = target;
            }
            target = get_next(target);

            PP_Token* val = calloc(1, sizeof(PP_Token));
            val->kind = PTK_NUM;
            val->val = find_macro(ident, macro) ? 1 : 0;

            cur->next = val;
            val->next = target;
            cur = target;
        } else {

            cur = get_next(cur);
        }
    }
    return head.next;
}
