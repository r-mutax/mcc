#ifndef MCC_INC_H
#define MCC_INC_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    NO_OPTION = 0,
    INCLUDE_PATH,
    OUTPUT_FILE,
    SRC_FILE
} MCC_OPTION;

typedef struct ARG_VEC ARG_VEC;
struct ARG_VEC{
    char* str;
    int len ;
    ARG_VEC* next;
};

#endif