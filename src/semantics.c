#include "semantics.h"
#include "errormsg.h"

static void check_function(Node* func);
static void check_compound_stmt(Node* cmpd_stmt);

bool semantics_check(Program* prog){

    Node* cur = prog->func_list;
    while(cur){
        check_function(cur);
        cur = cur->next;
    }
}

// --------------------------------------------------------
static void check_function(Node* func){
    if(func->kind != ND_FUNCTION){
        error("expect function node.");
    }

    if(func->is_declare) return;

    check_compound_stmt(func->body);
}

static void check_compound_stmt(Node* cmpd_stmt){
    Node* cur = cmpd_stmt->body;

    while(cur){
        cur = cur->next;
    }
}
