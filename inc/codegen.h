#ifndef CODEGEN_INC_H
#define CODEGEN_INC_H
#include "node.h"

void gen_compound_stmt(Node* node);
void gen(Node* node);
void gen_printline(char* p);
void gen_epilogue(void);
void gen_program(Program* prog);
#endif
