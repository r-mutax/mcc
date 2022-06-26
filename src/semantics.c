#include "semantics.h"
#include "errormsg.h"

static void check_function(Node* func);
static void check_compound_stmt(Node* cmpd_stmt);
static void check_stmt(Node* stmt);
static void check_expr(Node* expr);

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
        check_stmt(cur);
        cur = cur->next;
    }
}

static void check_stmt(Node* stmt){
    
    switch(stmt->kind){
        case ND_RETURN:
            check_expr(stmt->lhs);
            return;
        case ND_WHILE:
            check_expr(stmt->cond);
            check_stmt(stmt->body);
            return;
        case ND_FOR:
            check_expr(stmt->init);
            check_expr(stmt->cond);
            check_stmt(stmt->body);
            check_expr(stmt->iter);
            return;
        case ND_IF:
            check_expr(stmt->cond);
            check_stmt(stmt->body);
            if(stmt->else_body)
                check_stmt(stmt->else_body);
            return;
        case ND_DOWHILE:
            check_stmt(stmt->body);
            check_expr(stmt->cond);
            return;
        case ND_CMPDSTMT:
            check_compound_stmt(stmt);
            return;
        case ND_LABEL:
        case ND_GOTO:
        case ND_CONTINUE:
        case ND_CASE:
        case ND_BREAK:
            return;
        case ND_SWITCH:
            check_expr(stmt->cond);
            check_compound_stmt(stmt->body);
            return;
        case ND_DEFAULT:
        case ND_VOID_STMT:
            break;
        default:
            check_expr(stmt);
            break;
    }
}

static void check_expr(Node* expr){
    
    if(expr == NULL)
        return;

    switch(expr->kind){
        case ND_NUM:
            return;
        case ND_LVAR:
        case ND_GVAR:
        case ND_MEMBER:
            return;
        case ND_CALL:
            for(Node* cur = expr->arg; cur; cur = cur->next){
                check_expr(cur);
            }
            return;
        case ND_ASSIGN:
            check_expr(expr->rhs);
            return;
        case ND_ADDR:
            return;
        case ND_DREF:
            check_expr(expr->lhs);
            return;
        case ND_COMMA:
            check_expr(expr->lhs);
            check_expr(expr->rhs);
            return;
        case ND_AND:
            check_expr(expr->lhs);
            check_expr(expr->rhs);
            return;
        case ND_OR:
            check_expr(expr->lhs);
            check_expr(expr->rhs);
            return;
        case ND_COND_EXPR:
            check_expr(expr->cond);
            check_expr(expr->lhs);
            check_expr(expr->rhs);
            return;
    }

    check_expr(expr->lhs);
    check_expr(expr->rhs);
}
