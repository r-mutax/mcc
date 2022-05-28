#include "mcc.h"
#include "codegen.h"
#include "errormsg.h"

// grobal value -----------------------------------
static int label = 0;
static int while_label = 0;
static int if_label = 0;
static int for_label = 0;
static const char *argreg8[] = {"dil", "sil", "dl", "cl", "r8b", "r9b"};
static const char *argreg16[] = {"di", "si", "dx", "cx", "r8w", "r9w"};
static const char *argreg32[] = {"edi", "esi", "edx", "ecx", "r8d", "r9d"};
static const char *argreg64[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

// local function forward declaration. ------------
static void gen_lval(Node* node);
static void gen_stmt(Node* node);
static void gen_function(Node* func);
static void gen_grobal_variable(Node* node);
static void gen_compound_stmt(Node* node);
static void gen_printline(char* p);
static void gen_print_sym(Symbol* sym);
static void gen_epilogue(void);
static void gen(Node* node);

void gen_program(Program* prog){

    printf(".intel_syntax noprefix\n");

    // string constant output
    Symbol* sym = prog->string_list;
    while(sym){
        printf("  .data\n");
        gen_print_sym(sym);
        printf("  .string \"%s\"\n", sym->str);
        sym = sym->next;
    }

    // data / text section print
    Node* cur = prog->func_list;
    while(cur){
        if(cur->kind == ND_FUNCTION){
            gen_function(cur);
        } else {
            gen_grobal_variable(cur);
        }
        cur = cur->next;
    }
}

static void gen_print_sym(Symbol* sym){
    printf("%s:\n", sym->name);
}

static void gen_grobal_variable(Node* node){

    Node* cur = node->body;
    while(cur){
        printf("  .data\n");

        gen_print_sym(cur->sym);
        int size = cur->sym->type->size;
        if(cur->sym->type->pointer_to){
            size *= cur->sym->type->array_len;
        }
        printf("  .zero %d\n", size);
        cur = cur->next;
    }
}

static void gen_function(Node* func){

    if(func->is_declare){
        return;
    }

    printf("  .global %s\n", func->func_sym->name);
    printf("  .text\n");
    gen_print_sym(func->func_sym);

    // prologue
    printf("  push rbp\n");
    printf("  mov rbp, rsp\n");

    // alloc local variable area.
    int size = func->stack_size;
    size = ((size + 15) / 16) * 16;
    printf("  sub rsp, %d\n", size);

    // move arguments register to stack.
    Symbol* param = func->param;
    for(int argn = 0; argn < func->paramnum; argn++){
        if(param->type->size == 1) {
            printf("  mov [rbp - %d],%s\n", param->offset, argreg8[argn]);
        }else if(param->type->size == 2){
            printf("  mov [rbp - %d],%s\n", param->offset, argreg16[argn]);
        }else if(param->type->size == 4){
            printf("  mov [rbp - %d],%s\n", param->offset, argreg32[argn]);
        } else if(param->type->size == 8){
            printf("  mov [rbp - %d],%s\n", param->offset, argreg64[argn]);
        }
        param = param->next;
    }

    gen_compound_stmt(func->body->body);

    gen_epilogue();
}

static void gen_compound_stmt(Node* node){
    Node* cur = node;
    while(cur){
        if(cur->kind != ND_DECLARE){
            gen_printline(cur->line);
            gen_stmt(cur);
        }
        cur = cur->next;
    }
}

static void gen_stmt(Node* node){

    /* 
        [CAUTION]
            When end of stmt, aligment stack address to 16byte.
    */
    switch(node->kind){
        case ND_RETURN:
            gen(node->lhs);
            printf("  pop rax\n");
            gen_epilogue();
            return;
        case ND_WHILE:
            {
                int label = while_label++;
                printf(".LBegin%d:\n", label);
                gen(node->cond);
                printf("  pop rax\n");
                printf("  cmp rax, 0\n");
                printf("  je .LEnd%d\n", label);
                gen_stmt(node->body);
                printf("  jmp .LBegin%d\n", label);
                printf(".LEnd%d:\n", label);
                while_label++;
                return;                
            }
        case ND_FOR:
            {
                int label = for_label++;
                gen(node->init);
                printf("  pop rax\n");
                printf(".LBeginFor%d:\n", label);
                gen(node->cond);
                printf("  pop rax\n");
                printf("  cmp rax, 0\n");
                printf("  je .LEndFor%d\n", label);
                gen_stmt(node->body);
                gen(node->iter);
                printf("  pop rax\n");
                printf("  jmp .LBeginFor%d\n", label);
                printf(".LEndFor%d:\n", label);
                return;
            }
        case ND_IF:
        {
            int label = if_label++;
            gen(node->cond);
            printf("  pop rax\n");
            printf("  cmp rax, 0\n");

            if(node->else_body){
                printf("  je .LIfElse%d\n", label);
            } else {
                printf("  je .LIfEnd%d\n", label);
            }

            // print true-body
            gen_stmt(node->body);
            printf("  jmp .LIfEnd%d\n", label);
            
            if(node->else_body){
                printf(".LIfElse%d:\n", label);
                gen_stmt(node->else_body);
            }
            printf(".LIfEnd%d:\n", label);
            return;
        }
        case ND_CMPDSTMT:
            gen_compound_stmt(node->body);
            return;
        default:
            gen(node);
            printf("  pop rax\n");
            return;
    }
}

// generate code with 
static void gen(Node* node){

    if(node == NULL){
        return;
    }

    switch(node->kind){
        case ND_NUM:
            printf("  push %d\n", node->val);
            return;
        case ND_LVAR:
        case ND_GVAR:
        case ND_MEMBER:
            gen_lval(node);
            if(node->type->kind != TY_ARRAY){
                printf("  pop rax\n");
                if(node->type->size == 1){
                    printf("  movsx eax, BYTE PTR [rax]\n");
                }else if(node->type->size == 2){
                    printf("  movsx eax, WORD PTR [rax]\n");
                }else if(node->type->size == 4){
                    printf("  mov eax, [rax]\n");
                } else if(node->type->size == 8){
                    printf("  mov rax, [rax]\n");
                }
                printf("  push rax\n");
            }
            return;
        case ND_CALL:
            {
                int nargs = 0;
                for(Node* cur = node->arg; cur; cur = cur->next){
                    gen(cur);
                    nargs++;
                }

                for(;nargs; nargs--){
                    printf("  pop %s\n", argreg64[nargs - 1]);
                }

                printf("  call %s\n", node->sym->name);
                printf("  push rax\n");
                return ;
            }
        case ND_ASSIGN:
            gen_lval(node->lhs);
            gen(node->rhs);

            printf("  pop rdi\n");
            printf("  pop rax\n");
            if(node->lhs->type->size == 1){
                printf("  mov [rax], dil\n");
            } else if(node->lhs->type->size == 2){
                printf("  mov [rax], di\n");
            } else if(node->lhs->type->size == 4){
                printf("  mov [rax], edi\n");
            } else if(node->lhs->type->size == 8){
                printf("  mov [rax], rdi\n");
            }
            printf("  push rdi\n");
            return;
        case ND_ADDR:
            gen_lval(node->lhs);
            return;
        case ND_DREF:
            gen(node->lhs);
            printf("  pop rax\n");
            if(node->type->size == 1){
                printf("  movsbq rax, BYTE PTR [rax]\n");
            } else if(node->type->size == 2){
                printf("  movswq rax, WORD PTR [rax]\n");
            } else if(node->type->size == 4){
                printf("  mov eax, [rax]\n");
            } else if(node->type->size == 8){
                printf("  mov rax, [rax]\n");
            }
            printf("  push rax\n");
            return;
        case ND_COMMA:
            gen(node->lhs);
            printf("  pop rax\n");
            gen(node->rhs);
            return;
        case ND_AND:
        {
            int c = label++;
            gen(node->lhs);
            printf("  pop rax\n");
            printf("  cmp rax, 0\n");
            printf("  je .L.false%d\n", c);
            gen(node->rhs);
            printf("  pop rax\n");
            printf("  cmp rax, 0\n");
            printf("  je .L.false%d\n", c);
            printf("  mov rax, 1\n");
            printf("  jmp .L.End%d\n", c);
            printf(".L.false%d:\n", c);
            printf("  mov rax, 0\n");
            printf(".L.End%d:\n", c);
            printf("  push rax\n");
            return;
        }
        case ND_OR:
        {
            int c = label++;
            gen(node->lhs);
            printf("  pop rax\n");
            printf("  cmp rax, 0\n");
            printf("  jne .L.true%d\n", c);
            gen(node->rhs);
            printf("  pop rax\n");
            printf("  cmp rax, 0\n");
            printf("  jne .L.true%d\n", c);
            printf("  mov rax, 0\n");
            printf("  jmp .L.End%d\n", c);
            printf(".L.true%d:\n", c);
            printf("  mov rax, 1\n");
            printf(".L.End%d:\n", c);
            printf("  push rax\n");
            return;
        }
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
        case ND_MOD:
            printf("  cqo\n");
            printf("  idiv rdi\n");
            printf("  mov rax, rdx\n");
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
        case ND_BIT_AND:
            printf("  and rax, rdi\n");
            break;
        case ND_BIT_XOR:
            printf("  xor rax, rdi\n");
            break;
        case ND_BIT_OR:
            printf("  or rax, rdi\n");
            break;
    }

    printf("  push rax\n");
    return;
}

static void gen_printline(char* p){

    if(p == NULL) return;

    char* semi = strchr(p, ';');
    char* nl = strchr(p, '\n') - 1;

    char* pos = semi < nl ? semi : nl;

    char* line = calloc(pos - p + 1, sizeof(char));

    strncpy(line, p, pos - p + 1);
    printf("# %s\n", line);
    free(line);
}

static void gen_epilogue(void){
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
}

// local function ---------------------------------
static void gen_lval(Node* node){

    switch (node->kind){
        case ND_LVAR:
            printf("  mov rax, rbp\n");
            printf("  sub rax, %d\n", node->offset);
            printf("  push rax\n");
            return;
        case ND_GVAR:
            printf("  mov rax, OFFSET FLAT:%s\n", node->sym->name);
            printf("  push rax\n");
            return;
        case ND_DREF:
            gen(node->lhs);
            return;
        case ND_MEMBER:
            gen_lval(node->lhs);
            printf("  pop rax\n");
            printf("  add rax, %d\n", node->offset);
            printf("  push rax\n");
            return;        
    }

    error("Not a Variable.\n");
}

