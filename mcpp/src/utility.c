#include "mcpp.h"

char* strdup(char* src){
    int len = strlen(src);
    char* p = calloc(1, len + 1);
    memcpy(p, src, len);
    return p;
}