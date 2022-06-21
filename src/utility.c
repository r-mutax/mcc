#include "utility.h"

char* strndup(char* src, size_t n){
    char* p = calloc(1, n);
    memcpy(p, src, n);
    return p;
}

char* strdup(char* src){
    char* p = calloc(1, strlen(src));
    memcpy(p, src, strlen(src));
    return p;
}