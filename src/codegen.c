#include "mcc.h"
#include "codegen.h"
#include "errormsg.h"

// grobal value -----------------------------------
static int while_label = 0;
static int if_label = 0;
static int for_label = 0;

// local function forward declaration. ------------
static void gen_lval(Node* node);
static void gen_stmt(Node* node);

void gen_compound_stmt(Node* node){
    Node* cur = node;
    while(cur){
        gen_printline(cur->line);
        gen_stmt(cur);
        cur = cur->next;
        printf("  pop rax\n");
    }
}

void gen_stmt(Node* node){

    switch(node->kind){
        case ND_RETURN:
            gen(node->lhs);
            printf("  pop rax\n");
            gen_epilogue();
            return;
        case ND_WHILE:
            printf(".LBegin%d:\n", while_label);
            gen(node->cond);
            printf("  pop rax\n");
            printf("  cmp rax, 0\n");
            printf("  je .LEnd%d\n", while_label);
            gen_stmt(node->body);
            printf("  jmp .LBegin%d\n", while_label);
            printf(".LEnd%d:\n", while_label);
            while_label++;
            return;
        case ND_FOR:
            gen(node->init);
            printf(".LBeginFor%d:\n", for_label);
            gen(node->cond);
            printf("  pop rax\n");
            printf("  cmp rax, 0\n");
            printf("  je .LEndFor%d\n", for_label);
            gen_stmt(node->body);
            gen(node->iter);
            printf("  jmp .LBeginFor%d\n", for_label);
            printf(".LEndFor%d:\n", for_label);
            for_label++;
            return;
        case ND_IF:
            gen(node->cond);
            printf("  pop rax\n");
            printf("  cmp rax, 0\n");

            if(node->else_body){
                printf("  je .LIfElse%d\n", if_label);
            } else {
                printf("  je .LIfEnd%d\n", if_label);
            }

            // print true-body
            gen_stmt(node->body);
            printf("  jmp .LIfEnd%d\n", if_label);
            
            if(node->else_body){
                printf(".LIfElse%d:\n", if_label);
                gen_stmt(node->else_body);
            }
            printf(".LIfEnd%d:\n", if_label);
            if_label++;
            return;
        case ND_CMPDSTMT:
            gen_compound_stmt(node->body);
            return;
        default:
            gen(node);
            return;
    }
}

// generate code with 
void gen(Node* node){

    if(node == NULL){
        return;
    }

    switch(node->kind){
        case ND_NUM:
            printf("  push %d\n", node->val);
            return;
        case ND_LVAR:
            gen_lval(node);
            printf("  pop rax\n");
            printf("  mov rax, [rax]\n");
            printf("  push rax\n");
            return;
        case ND_CALL:
            {
                char* fname = calloc(node->len, sizeof(char));
                strncpy(fname, node->func_name, node->len);
                printf("  call %s\n", fname);
                free(fname);
                printf("  push rax\n");
                return ;
            }
        case ND_ASSIGN:
            gen_lval(node->lhs);
            gen(node->rhs);

            printf("  pop rdi\n");
            printf("  pop rax\n");
            printf("  mov [rax], rdi\n");
            printf("  push rdi\n");
            return;
    }

    if(node->kind == ND_NUM){
        printf("  push %d\n", node->val);
        return;
    }

    gen(node->lhs);
    gen(node->rhs);

    printf("  pop rdi\n");
    printf("  pop rax\n");

    switch(node->kind){
        case ND_ADD:
            printf("  add rax, rdi\n");
            break;
        case ND_SUB:
            printf("  sub rax, rdi\n");
            break;
        case ND_MUL:
            printf("  imul rax, rdi\n");
            break;
        case ND_DIV:
            printf("  cqo\n");
            printf("  idiv rdi\n");
            break;
        case ND_EQ:
            printf("  cmp rax, rdi\n");
            printf("  sete al\n");
            printf("  movzb rax, al\n");
            break;
        case ND_NE:
            printf("  cmp rax, rdi\n");
            printf("  setne al\n");
            printf("  movzb rax, al\n");
            break;
        case ND_LT:
            printf("  cmp rax, rdi\n");
            printf("  setl al\n");
            printf("  movzb rax, al\n");
            break;
        case ND_LE:
            printf("  cmp rax, rdi\n");
            printf("  setle al\n");
            printf("  movzb rax, al\n");
            break;
    }

    printf("  push rax\n");
    return;
}

void gen_printline(char* p){

    char* pos = strchr(p, ';');
    char* line = calloc(pos - p + 1, sizeof(char));

    strncpy(line, p, pos - p + 1);
    printf("# %s\n", line);
    free(line);
}

void gen_epilogue(void){
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
}

// local function ---------------------------------
static void gen_lval(Node* node){
    if(node->kind != ND_LVAR){
        error("expect local variable, but get another node.");
    }

    printf("  mov rax, rbp\n");
    printf("  sub rax, %d\n", node->offset);
    printf("  push rax\n");
}

