#include "utility.h"

char* strndup(char* src, size_t n){
    char* p = calloc(1, n + 10);
    memcpy(p, src, n);
    return p;
}

char* strdup(char* src){
    char* p = calloc(1, strlen(src) + 10);
    memcpy(p, src, strlen(src));
    return p;
}