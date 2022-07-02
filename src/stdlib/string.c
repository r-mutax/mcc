#include "string.h"

void memcpy(void* s1, void* s2, int n){
    char* dst = (char*)s1;
    char* src = (char*)s2;

    for(int i = 0; i < n; i++){
        dst = src;
        dst++;
        src++;
    }
    return;
}

void memmove(void* s1, void* s2, int n){
    char* data[256];
    char* dst = s1;
    char* src = s2;

    memcpy(data, src, n);
    memcpy(dst, data, n);
    return;
}

char* strcpy(char* s1, char* s2){
    char* ret = s1;
    int flg = 1;
    do {
        if(*s2 == 0) flg = 0;
        s1 = s2;
        s1++;
        s2++;

    } while(flg);
    return ret;
}