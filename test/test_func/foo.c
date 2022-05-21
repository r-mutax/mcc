#include <stdlib.h>
#include <stdio.h>

int exit_error(char* p){
    fprintf(stderr, "%s", p);
    exit(1);
}

int add(int a, int b){
    return a + b;
}

int foo(void){
    return 5;
}

void print_fizz(){
    printf("Fizz ");
}

void print_bazz(){
    printf("Bazz ");
}

void print_fizz_bazz(){
    printf("FizzBazz ");
}

void print_int(int a){
    printf("%d ", a);
}

void tmalloc(long** p){
    long *q = malloc(sizeof(long) * 4);
    q[0] = 1;
    q[1] = 2;
    q[2] = 3;
    q[3] = 4;
    
    *p = q;
}