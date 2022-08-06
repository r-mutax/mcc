#include "mcpp.h"

char* strndup(char* src, size_t n){
    char* p = calloc(1, n + 1);
    memcpy(p, src, n);
    return p;
}

char* strdup(char* src){
    int len = strlen(src);
    char* p = calloc(1, len + 1);
    memcpy(p, src, len);
    return p;
}