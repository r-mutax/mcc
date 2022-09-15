#include "mcc1.h"
#include "codegen.h"
#include "errormsg.h"

// grobal value -----------------------------------
static Node* cur_function;
static int g_label = 0;
static int g_label_stack[1000];
static int g_stack_idx = -1;
static const char *argreg8[] = {"dil", "sil", "dl", "cl", "r8b", "r9b"};
static const char *argreg16[] = {"di", "si", "dx", "cx", "r8w", "r9w"};
static const char *argreg32[] = {"edi", "esi", "edx", "ecx", "r8d", "r9d"};
static const char *argreg64[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
extern FILE* output_file;

// cast instruction dst_src
static char i64_i8[] = "movsx rax, al";
static char i32_i8[] = "movsx eax, al";
static char i16_i8[] = "movsx ax, al";
static char i8_i16[] = "mov rdi, 0\n  mov dil, al\n  mov rax, rdi";
static char i32_i16[] = "movsx eax, ax";
static char i64_i16[] = "movsx rax, ax";
static char i8_i32[] = "mov rdi, 0\n  mov dil, al\n  mov rax, rdi";
static char i16_i32[] = "mov rdi, 0\n  mov di, ax\n  mov rax, rdi";
static char i32_i64[] = "mov eax, eax";
static char i64_i32[] = "movsx rax, eax";
static char i8_i64[] = "mov rdi, 0\n  mov dil, al\n  mov rax, rdi";
static char i16_i64[] = "mov rdi, 0\n  mov di, ax\n  mov rax, rdi";

static char u32_i8[] = "movzx eax, al";
static char u32_i16[] = "movzx eax, ax";
static char u32_i64[] = "mov eax, eax";


// cast instruction table
// column : source, row : destination
static char* cast_table[][8] = {
    //  i8       i16       i32       i64       u8       u16       u32       u64
    {   NULL,    i8_i16,   i8_i32,   i8_i64,   i32_i8,  i32_i8,   i32_i8,   i32_i8   },  // i8
    {   i16_i8,  NULL,     i16_i32,  i16_i64,  u32_i8,  i32_i16,  i32_i16,  i32_i16  },  // i16
    {   i32_i8,  i32_i16,  NULL,     NULL,     u32_i8,  u32_i16,  NULL,     i32_i64  },  // i32
    {   i64_i8,  i64_i16,  i64_i32,  NULL,     u32_i8,  u32_i16,  NULL,     NULL     },  // i64
    {   u32_i8,  u32_i8,   u32_i8,   u32_i8,   NULL,    u32_i8,   u32_i8,   u32_i8   },  // u8
    {   u32_i8,  u32_i16,  u32_i16,  u32_i16,  u32_i8,  u32_i16,  u32_i16,  u32_i16  },  // u16
    {   u32_i8,  u32_i16,  NULL,     u32_i64,  u32_i8,  u32_i16,  NULL,     u32_i64  },  // u32
    {   u32_i8,  u32_i16,  NULL,     NULL,     u32_i8,  u32_i16,  NULL,     NULL     }   // u64
};

// local function forward declaration. ------------
static void gen_lval(Node* node);
static void gen_stmt(Node* node);
static void gen_function(Node* func);
static void gen_grobal_variable(Node* node);
static void gen_compound_stmt(Node* node);
static void gen_printline(Token* p);
static void gen_print_sym(Symbol* sym);
static void gen_epilogue(void);
static void gen(Node* node);
static void gen_cast(Type* from, Type* to);
static SIZE_TYPE_ID get_size_type_id(Type* ty);

void gen_program(Program* prog){

    fprintf(output_file, ".intel_syntax noprefix\n");

    // string constant output
    fprintf(output_file, "  .data\n");
    Symbol* sym = prog->string_list;
    while(sym){
        gen_print_sym(sym);
        fprintf(output_file, "  .string \"%s\"\n", sym->str);
        sym = sym->next;
    }

    // data section output
    Node    tmp;
    memset(&tmp, 0, sizeof(Node));
    sym = prog->data_list;
    while(sym){
        tmp.kind = ND_DECLARE;
        tmp.sym = sym;
        gen_grobal_variable(&tmp);
        sym = sym->next;
    }


    // text section output
    Node* cur = prog->func_list;
    while(cur){
        if(cur->kind == ND_FUNCTION){
            gen_function(cur);
        }
        cur = cur->next;
    }
}

static void gen_print_sym(Symbol* sym){
    if(sym->is_static){
        if(sym->func_name){
            fprintf(output_file, ".L%s_%s:\n", sym->func_name, sym->name);
        } else {
            fprintf(output_file, ".L%s:\n", sym->name);
        }
    } else {
        fprintf(output_file, "%s:\n", sym->name);
    }
}

static void gen_grobal_variable(Node* node){

    gen_print_sym(node->sym);
    int size = node->sym->type->size;
    if(node->sym->type->pointer_to){
        size *= node->sym->type->array_len;
    }
    fprintf(output_file, "  .zero %d\n", size);
}

static void gen_function(Node* func){

    if(func->is_declare){
        return;
    }

    cur_function = func;

    fprintf(output_file, "  .global %s\n", func->func_sym->name);
    fprintf(output_file, "  .text\n");
    gen_print_sym(func->func_sym);

    // prologue
    fprintf(output_file, "  push rbp\n");
    fprintf(output_file, "  mov rbp, rsp\n");

    // alloc local variable area.
    int size = func->stack_size;
    size = ((size + 15) / 16) * 16;
    fprintf(output_file, "  sub rsp, %d\n", size);

    // move arguments register to stack.
    Symbol* param = func->param;
    for(int argn = 0; argn < func->paramnum; argn++){
        if(param->type->size == 1) {
            fprintf(output_file, "  mov [rbp - %d],%s\n", param->offset, argreg8[argn]);
        }else if(param->type->size == 2){
            fprintf(output_file, "  mov [rbp - %d],%s\n", param->offset, argreg16[argn]);
        }else if(param->type->size == 4){
            fprintf(output_file, "  mov [rbp - %d],%s\n", param->offset, argreg32[argn]);
        } else if(param->type->size == 8){
            fprintf(output_file, "  mov [rbp - %d],%s\n", param->offset, argreg64[argn]);
        }
        param = param->next;
    }

    gen_compound_stmt(func->body->body);

    gen_epilogue();

    cur_function = NULL;
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

static void gen_cast(Type* from, Type* to){
    SIZE_TYPE_ID src_i = get_size_type_id(from);
    SIZE_TYPE_ID dst_i = get_size_type_id(to);

    fprintf(output_file, "  pop rax\n");

    if(cast_table[dst_i][src_i]){
        fprintf(output_file, "  %s\n", cast_table[dst_i][src_i]);
    }
}

static void gen_stmt(Node* node){

    /* 
        [CAUTION]
            When end of stmt, aligment stack address to 16byte.
    */
    switch(node->kind){
        case ND_RETURN:
            if(node->lhs){
                gen(node->lhs);
                fprintf(output_file, "  pop rax\n");
            } else {
                fprintf(output_file, "  nop\n");
            }
            gen_epilogue();
            return;
        case ND_WHILE:
            {
                int label = g_label++;
                g_label_stack[++g_stack_idx] = label;
                fprintf(output_file, ".LBegin_%d:\n", label);
                fprintf(output_file, ".LLoop_%d:\n", label);
                gen(node->cond);
                fprintf(output_file, "  pop rax\n");
                fprintf(output_file, "  cmp rax, 0\n");
                fprintf(output_file, "  je .LEnd_%d\n", label);
                gen_stmt(node->body);
                fprintf(output_file, "  jmp .LBegin_%d\n", label);
                fprintf(output_file, ".LEnd_%d:\n", label);
                g_stack_idx--;
                return;                
            }
        case ND_FOR:
            {
                int label = g_label++;
                g_label_stack[++g_stack_idx] = label;
                if(node->init){
                    if(node->init->kind == ND_CMPDSTMT){
                        gen_stmt(node->init);
                    } else {
                        gen(node->init);
                        fprintf(output_file, "  pop rax\n");
                    }
                }
                fprintf(output_file, ".LBegin_%d:\n", label);

                if(node->cond){
                    gen(node->cond);
                    fprintf(output_file, "  pop rax\n");
                } else {
                    fprintf(output_file, "  mov rax, 1\n");
                }
                fprintf(output_file, "  cmp rax, 0\n");
                fprintf(output_file, "  je .LEnd_%d\n", label);
                gen_stmt(node->body);
                fprintf(output_file, ".LLoop_%d:\n", label);
                if(node->iter){
                    gen(node->iter);
                    fprintf(output_file, "  pop rax\n");
                }
                fprintf(output_file, "  jmp .LBegin_%d\n", label);
                fprintf(output_file, ".LEnd_%d:\n", label);
                g_stack_idx--;
                return;
            }
        case ND_IF:
        {
            int label = g_label++;
            gen(node->cond);
            fprintf(output_file, "  pop rax\n");
            fprintf(output_file, "  cmp rax, 0\n");

            if(node->else_body){
                fprintf(output_file, "  je .LIfElse%d\n", label);
            } else {
                fprintf(output_file, "  je .LIfEnd%d\n", label);
            }

            // print true-body
            gen_stmt(node->body);
            fprintf(output_file, "  jmp .LIfEnd%d\n", label);
            
            if(node->else_body){
                fprintf(output_file, ".LIfElse%d:\n", label);
                gen_stmt(node->else_body);
            }
            fprintf(output_file, ".LIfEnd%d:\n", label);
            return;
        }
        case ND_DOWHILE:
        {
            int label = g_label++;
            g_label_stack[++g_stack_idx] = label;
            fprintf(output_file, ".LBegin_%d:\n", label);
            gen_stmt(node->body);
            fprintf(output_file, ".LLoop_%d:\n", label);
            gen(node->cond);
            fprintf(output_file, "  pop rax\n");
            fprintf(output_file, "  cmp rax, 0\n");
            fprintf(output_file, "  jne .LBegin_%d\n", label);
            fprintf(output_file, ".LEnd_%d:\n",label);
            g_stack_idx--;
            return;
        }
        case ND_CMPDSTMT:
            gen_compound_stmt(node->body);
            return;
        case ND_LABEL:
            fprintf(output_file, ".L%s_%s:\n", cur_function->func_sym->name, node->label_name);
            return;
        case ND_GOTO:
            fprintf(output_file, "  jmp .L%s_%s\n", cur_function->func_sym->name, node->label_name);
            return;
        case ND_CONTINUE:
            if(g_stack_idx == -1){
                error("can't use continue in this place.\n");
            }
            fprintf(output_file, "  jmp .LLoop_%d\n", g_label_stack[g_stack_idx]);
            return;
        case ND_CASE:
            if(g_stack_idx == -1){
                error("case label not with in switch statement.\n");
            }
            fprintf(output_file, ".LCase%d_%lu:\n", g_label_stack[g_stack_idx], node->lhs->val);
            return;
        case ND_BREAK:
            if(g_stack_idx == -1){
                error("can't use break in this place.\n");
            }
            fprintf(output_file, "  jmp .LEnd_%d\n", g_label_stack[g_stack_idx]);
            return;
        case ND_SWITCH:
        {
            int label = g_label++;
            g_label_stack[++g_stack_idx] = label;
            gen(node->cond);
            fprintf(output_file, "  pop rax\n");
            Node* cur_case = node->case_label;
            while(cur_case){
                fprintf(output_file, "  cmp rax, %lu\n", cur_case->val);
                fprintf(output_file, "  je .LCase%d_%lu\n", label, cur_case->val);
                cur_case = cur_case->next;
            }
            if(node->default_label)
                fprintf(output_file, "  jmp .LDefault_%d\n", label);

            fprintf(output_file, "  jmp .LEnd_%d\n", label);
            gen_compound_stmt(node->body);
            fprintf(output_file, ".LEnd_%d:\n", label);
            g_stack_idx--;
            return;
        }
        case ND_DEFAULT:
            fprintf(output_file, ".LDefault_%d:\n", g_label_stack[g_stack_idx]);
            return;
        case ND_VOID_STMT:
            return;
        default:
            gen(node);
            fprintf(output_file, "  pop rax\n");
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
            fprintf(output_file, "  mov rax, %lu\n", node->val);
            fprintf(output_file, "  push rax\n");
            return;
        case ND_LVAR:
        case ND_GVAR:
        case ND_MEMBER:
            gen_lval(node);
            if(node->type->kind != TY_ARRAY){
                fprintf(output_file, "  pop rax\n");
                if(node->type->size == 1){
                    fprintf(output_file, "  movsx eax, BYTE PTR [rax]\n");
                }else if(node->type->size == 2){
                    fprintf(output_file, "  movsx eax, WORD PTR [rax]\n");
                }else if(node->type->size == 4){
                    fprintf(output_file, "  mov eax, [rax]\n");
                } else if(node->type->size == 8){
                    fprintf(output_file, "  mov rax, [rax]\n");
                }
                fprintf(output_file, "  push rax\n");
            }
            return;
        case ND_CALL:
            {
                int nargs = 0;
                Symbol* param = node->sym->args;
                for(Node* cur = node->arg; cur; cur = cur->next, param = param->next){
                    gen(cur);

                    // implicit cast at function arguments.
                    gen_cast(cur->type, param->type);
                    fprintf(output_file, "  push rax\n");
                    nargs++;
                }

                for(;nargs; nargs--){
                    fprintf(output_file, "  pop %s\n", argreg64[nargs - 1]);
                }

                fprintf(output_file, "  call %s\n", node->sym->name);
                fprintf(output_file, "  push rax\n");
                return ;
            }
        case ND_ASSIGN:
            gen_lval(node->lhs);
            gen(node->rhs);

            fprintf(output_file, "  pop rdi\n");
            fprintf(output_file, "  pop rax\n");

            if(node->lhs->type->kind == TY_BOOL){
                fprintf(output_file, "  cmp rdi, 0\n");
                fprintf(output_file, "  setne dil\n");
            }

            if(node->lhs->type->size == 1){
                fprintf(output_file, "  mov [rax], dil\n");
            } else if(node->lhs->type->size == 2){
                fprintf(output_file, "  mov [rax], di\n");
            } else if(node->lhs->type->size == 4){
                fprintf(output_file, "  mov [rax], edi\n");
            } else if(node->lhs->type->size == 8){
                fprintf(output_file, "  mov [rax], rdi\n");
            }
            fprintf(output_file, "  push rdi\n");
            return;
        case ND_ADDR:
            gen_lval(node->lhs);
            return;
        case ND_DREF:
            gen(node->lhs);
            fprintf(output_file, "  pop rax\n");
            if(node->type->size == 1){
                fprintf(output_file, "  movsbq rax, BYTE PTR [rax]\n");
            } else if(node->type->size == 2){
                fprintf(output_file, "  movswq rax, WORD PTR [rax]\n");
            } else if(node->type->size == 4){
                fprintf(output_file, "  mov eax, [rax]\n");
            } else if(node->type->size == 8){
                fprintf(output_file, "  mov rax, [rax]\n");
            }
            fprintf(output_file, "  push rax\n");
            return;
        case ND_COMMA:
            gen(node->lhs);
            fprintf(output_file, "  pop rax\n");
            gen(node->rhs);
            return;
        case ND_AND:
        {
            int c = g_label++;
            gen(node->lhs);
            fprintf(output_file, "  pop rax\n");
            fprintf(output_file, "  cmp rax, 0\n");
            fprintf(output_file, "  je .L.false%d\n", c);
            gen(node->rhs);
            fprintf(output_file, "  pop rax\n");
            fprintf(output_file, "  cmp rax, 0\n");
            fprintf(output_file, "  je .L.false%d\n", c);
            fprintf(output_file, "  mov rax, 1\n");
            fprintf(output_file, "  jmp .L.End%d\n", c);
            fprintf(output_file, ".L.false%d:\n", c);
            fprintf(output_file, "  mov rax, 0\n");
            fprintf(output_file, ".L.End%d:\n", c);
            fprintf(output_file, "  push rax\n");
            return;
        }
        case ND_OR:
        {
            int c = g_label++;
            gen(node->lhs);
            fprintf(output_file, "  pop rax\n");
            fprintf(output_file, "  cmp rax, 0\n");
            fprintf(output_file, "  jne .L.true%d\n", c);
            gen(node->rhs);
            fprintf(output_file, "  pop rax\n");
            fprintf(output_file, "  cmp rax, 0\n");
            fprintf(output_file, "  jne .L.true%d\n", c);
            fprintf(output_file, "  mov rax, 0\n");
            fprintf(output_file, "  jmp .L.End%d\n", c);
            fprintf(output_file, ".L.true%d:\n", c);
            fprintf(output_file, "  mov rax, 1\n");
            fprintf(output_file, ".L.End%d:\n", c);
            fprintf(output_file, "  push rax\n");
            return;
        }
        case ND_COND_EXPR:
        {
            int label = g_label++;
            gen(node->cond);
            fprintf(output_file, "  pop rax\n");
            fprintf(output_file, "  cmp rax, 0\n");
            fprintf(output_file, "  je .L.false%d\n", label);
            gen(node->lhs);
            fprintf(output_file, "  jmp .L.End%d\n", label);
            fprintf(output_file, ".L.false%d:\n", label);
            gen(node->rhs);
            fprintf(output_file, ".L.End%d:\n", label);
            return;
        }
        case ND_CAST:
            gen(node->lhs);
            gen_cast(node->lhs->type, node->type);
            if(node->type->kind == TY_BOOL){
                fprintf(output_file, "  cmp rax, 0\n");
                fprintf(output_file, "  setne al\n");
                fprintf(output_file, "  movzx rax, al\n");               
            }
            fprintf(output_file, "  push rax\n");
            return;
        case ND_NOT:
            gen(node->lhs);
            fprintf(output_file, "  pop rax\n");
            fprintf(output_file, "  cmp rax, 0\n");
            fprintf(output_file, "  sete al\n");
            fprintf(output_file, "  movzb eax, al\n");
            fprintf(output_file, "  push rax\n");
            return;
    }

    if(node->kind == ND_NUM){
        fprintf(output_file, "  mov rax, %lu\n", node->val);
        fprintf(output_file, "  push rax\n");
        return;
    }

    gen(node->lhs);
    gen(node->rhs);

    fprintf(output_file, "  pop rdi\n");
    fprintf(output_file, "  pop rax\n");

    switch(node->kind){
        case ND_ADD:
            fprintf(output_file, "  add rax, rdi\n");
            break;
        case ND_SUB:
            fprintf(output_file, "  sub rax, rdi\n");
            break;
        case ND_MUL:
            fprintf(output_file, "  imul rax, rdi\n");
            break;
        case ND_DIV:
            fprintf(output_file, "  cqo\n");
            fprintf(output_file, "  idiv rdi\n");
            break;
        case ND_MOD:
            fprintf(output_file, "  cqo\n");
            fprintf(output_file, "  idiv rdi\n");
            fprintf(output_file, "  mov rax, rdx\n");
            break;
        case ND_EQ:
            fprintf(output_file, "  cmp rax, rdi\n");
            fprintf(output_file, "  sete al\n");
            fprintf(output_file, "  movzb rax, al\n");
            break;
        case ND_NE:
            fprintf(output_file, "  cmp rax, rdi\n");
            fprintf(output_file, "  setne al\n");
            fprintf(output_file, "  movzb rax, al\n");
            break;
        case ND_LT:
            fprintf(output_file, "  cmp rax, rdi\n");
            fprintf(output_file, "  setl al\n");
            fprintf(output_file, "  movzb rax, al\n");
            break;
        case ND_LE:
            fprintf(output_file, "  cmp rax, rdi\n");
            fprintf(output_file, "  setle al\n");
            fprintf(output_file, "  movzb rax, al\n");
            break;
        case ND_BIT_AND:
            fprintf(output_file, "  and rax, rdi\n");
            break;
        case ND_BIT_XOR:
            fprintf(output_file, "  xor rax, rdi\n");
            break;
        case ND_BIT_OR:
            fprintf(output_file, "  or rax, rdi\n");
            break;
        case ND_BIT_LSHIFT:
            fprintf(output_file, "  mov rcx, rdi\n");
            fprintf(output_file, "  shl rax, cl\n");
            break;
        case ND_BIT_RSHIFT:
            fprintf(output_file, "  mov rcx, rdi\n");
            fprintf(output_file, "  shr rax, cl\n");
            break;
    }

    fprintf(output_file, "  push rax\n");
    return;
}

static void gen_printline(Token* tok){

    // if(tok == NULL) return;

    // char* semi = strchr(tok->str, ';');
    // char* nl = strchr(tok->str, '\n') - 1;

    // char* pos = semi < nl ? semi : nl;

    // char* line = calloc(pos - tok->str + 1, sizeof(char));

    // strncpy(line, tok->str, pos - tok->str + 1);
    // fprintf(output_file, "# %s\n", line);
    // free(line);
}

static void gen_epilogue(void){
    fprintf(output_file, "  mov rsp, rbp\n");
    fprintf(output_file, "  pop rbp\n");
    fprintf(output_file, "  ret\n");
}

// local function ---------------------------------
static void gen_lval(Node* node){

    switch (node->kind){
        case ND_LVAR:
            fprintf(output_file, "  mov rax, rbp\n");
            fprintf(output_file, "  sub rax, %d\n", node->offset);
            fprintf(output_file, "  push rax\n");
            return;
        case ND_GVAR:
            if(node->sym->is_static){
                if(cur_function){
                    fprintf(output_file, "  mov rax, OFFSET FLAT:.L%s_%s\n", cur_function->func_sym->name, node->sym->name);
                } else {
                    fprintf(output_file, "  mov rax, OFFSET FLAT:.L%s\n", node->sym->name);
                }
            } else {
                fprintf(output_file, "  mov rax, OFFSET FLAT:%s\n", node->sym->name);
            }
            fprintf(output_file, "  push rax\n");
            return;
        case ND_DREF:
            gen(node->lhs);
            return;
        case ND_MEMBER:
            gen_lval(node->lhs);
            fprintf(output_file, "  pop rax\n");
            fprintf(output_file, "  add rax, %d\n", node->offset);
            fprintf(output_file, "  push rax\n");
            return;        
    }

    error("Not a Variable.\n");
}

static SIZE_TYPE_ID get_size_type_id(Type* ty)
{
    SIZE_TYPE_ID id = 0;
    switch(ty->size){
        case 1:
            id = ty->is_unsigned ? u8 : i8;
            break;
        case 2:
            id = ty->is_unsigned ? u16 : i16;
            break;
        case 4:
            id = ty->is_unsigned ? u32 : i32;
            break;
        case 8:
            id = ty->is_unsigned ? u64 : i64;
            break;
        default:
            id = ierr;
            break;
    }

    return id;
}