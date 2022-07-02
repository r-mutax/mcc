#include "test.h"
#include "testlib.h"

#define TEST_DEF 1

long g_x;

void assert_c(char lhs, char rhs, char* p){
    if(lhs != rhs){
        print_int(lhs);
        print_int(rhs);

        exit_error(p);
    }
}

void assert_s(short lhs, short rhs, char* p){
    if(lhs != rhs){
        print_int(lhs);
        print_int(rhs);

        exit_error(p);
    }
}

void assert(int lhs, int rhs, char* p){
    if(lhs != rhs)
    {
        print_int(lhs);

        print_int(rhs);

        exit_error(p);
    }
}

void assert2(int lhs, int rhs, char* p){
    print_int(rhs);
    print_int(lhs);
    exit_error(p);
}

int test_static(){
    static int a;
    return a++;
}

int test_lvar(){
    long abc = 15;
    long b = 23;
    return abc + b;
}

int test_return(){
    return 5;
    4 * 5;
}

int test_add(int a, int b){
    return a + b;
}

void test_func(){

    assert(5, test_add(1, 4), "error : multi paramaters function call.\n");

    
    printf("function test is completed !\n");
}

void test_stmt(){

    assert(5, test_return(), "error : return.\n");

    int a = 0;
    int i = 0;
    int b = 0;
    while(a < 3) a = a + 1;
    assert(3, a, "error : while.\n");

    a = 0;
    if(5 == 5) a = 4;
    assert(4, a, "error : if - true path.\n");

    a = 0;
    if(4 == 5) a = 4;
    assert(0, a, "error : if - false path.\n");

    a = 0;
    if(5 == 5) a = 4;
    else a = 9;
    assert(4, a, "error : if - else, true path.\n");

    a = 0;
    if(4 == 5) a = 4;
    else a = 9;
    assert(9, a, "error : if - else, false path.\n");

    a = 0;
    if(4 == 4){
        a = 3;
        a = a + a;
    }
    assert(6, a, "error : if , compound stmt.\n");

    a = 0;
    for(i = 1; i < 3; i = i + 1){
        a = a + 1;
    }
    assert(2, a, "error : for stmt.\n");

    a = 0;
    goto label1;
    a = 10;
    label1:
    assert(0, a, "error : goto stmt.\n");

    a = 0;
    label2:
    a++;
    if(a == 10) goto label3;
    goto label2;
    label3:
    assert(10, a, "error : goto forward label.\n");
    
    int s = 15;
    switch(s){
        case 1:
            s = 25;
            break;
        case 225 / 5 / 9 * 3:
            s = s + s;
            break;
        case 20:
            s = s + 12;
            break;
        case 25:
            s = s * 4;
            break;
    }
    assert(30, s, "error : switch-case stmt.\n");

    int cnt = 0;
    while(1){
        cnt++;
        if(cnt > 10) assert(1, 0, "error : break stmt in while loop.\n");
        break;
    }

    for(cnt = 0; cnt < 10; cnt++){
        break;
    }
    assert(cnt, 0, "error : break stmt in for loop.\n");

    a = 10;
    switch(3){
        case 3 + 3 + 4 + 5 * 2:
            break;
        default:
            a = 20;
            break;
        case 2:
            break;
    }
    assert(a, 20, "error : default label.\n");

    a = 10;
    switch(3){
        case 1:
            a = 20;
            break;
        default:
            a = 25;
            break;
        case 1 + 2:
            a = 10;
            break;
    }
    assert(a, 10, "error : no default label.\n");

    a = 20;
    switch(2){
        case 1 ? 2 : 0:
            a = 30;
            break;
        default:
            a = 25;
            break;
    }
    assert(a, 30, "error : case label value is condition expression.\n");

    a = 0;
    do{
        a += 35;
    } while(0);
    assert(a, 35, "error : do-while statement.\n");

    a = 0;
    do {
        a += 1;
        if(a == 3) break;
    } while(a < 5);
    assert(a, 3, "error : break statement in do-while statement.\n");
    
    a = 0;
    b = 0;
    for(i = 0; i < 3; i++){
        a++;
        continue;
        b++;

    }
    assert(b, 0, "error : continue statement in for statement.\n");
   
    a = 0;
    b = 0;
    while(a < 3){
        a++;
        continue;
        b++;
    }
    assert(b, 0, "error : continue statement in while statement.\n");

    a = 0;
    b = 0;
    do {
        a++;
        continue;
        b++;
    } while(a < 3);
    assert(b, 0, "error : continue statement in do-while statement.\n");

    // void statement test
    ;;;;;;

    printf("stmt test is completed !\n");
}

void test_expr(){

    int a = 0;
    int b = 0;

    assert(42, 42, "error : numeric.\n");
    assert(15, 3 + 16 -    4, "error : add and sub.\n");
    assert(14, 4 + 2 * 5, "error : multiple.\n");
    assert(20, 25 - 10 / 2, "error : div.\n");
    assert(1, 5 == 5, "error : equal.\n");
    assert(35, (2 + 5) * 5, "error : bracket.\n");
    assert(20, (-10 + 20) * 2, "error : bracket 2.\n");
    assert(1, 5 == 5, "error : equal.\n");
    assert(0, 3 == 5, "error : equal 2.\n");
    assert(1, (5-3) != 6, "error : not equal.\n");
    assert(0, 4 * 2 != 8, "error : not equal 2.\n");
    assert(1, 5 > 4, "error : lt.\n");
    assert(0, 5 > 5, "error : lt 2.\n");
    assert(1, 4 >= 4, "error : le.\n");
    assert(1, 5 >= 4, "error : le 2.\n");
    assert(1, 4 < 5, "error : gt.\n");
    assert(1, 4 <= 4, "error : ge\n");
    assert(3, 10 % 7, "error : test mod\n");
    assert(3, 5 % 2 + 12 % 5, "error : test left associated.\n");

    assert(38, test_lvar(), "error : local variable.\n");
    a += 1;
    assert(1, a, "error : += operater.\n");

    a -= 2;
    assert(-1, a, "error : -= operater.\n");

    a *= -6;
    assert(6, a, "error : *= operater.\n");

    a /= 3;
    assert(2, a, "error : /= operater.\n");

    a = 12;
    a %= 5;
    assert(2, a, "error : %= operater.\n");

    a += 3 + 5;
    assert(10, a, "error : += operand, when rvalue is expr.\n");

    assert(5, (a = 3, a = a + 2), "error : comma operater.\n");

    a = 3;
    assert(4, ++a, "error : increment operater.\n");
    assert(3, --a, "error : comma operater.\n");

    assert(3, a++, "error : post-increment operater.\n");
    assert(4, a, "error : post-increment operater 2.\n");

    assert(4, a--, "error : post-decrement operater.\n");
    assert(3, a, "error : post-decrement operater.\n");

    assert(36, 100 & 44, "error : bitwise-AND.\n");
    assert(153, 115 ^ 234, "error : bitwise-Xor.\n");
    assert(126, 78 | 56, "error : bitwise-OR.\n");
    assert(31, 15 | 56 ^ 100 & 44, "error : priority of bitwise operater.\n");

    assert(23, 3 | 4 | 5 | 19, "error : multiple bitwise-Or.\n");

    assert(1, 1 && 44, "error : logical AND , both value is true.\n");
    assert(0, 0 && 44, "error : logical AND , lvalue is true.\n");
    assert(0, 1 && 0, "error : logical AND , rvalue is true.\n");
    assert(0, 0 && 0, "error : logical AND , both value is false.\n");

    assert(1, 1 || 44, "error : logical OR , both value is true.\n");
    assert(1, 0 || 44, "error : logical OR , lvalue is true.\n");
    assert(1, 1 || 0, "error : logical OR , rvalue is true.\n");
    assert(0, 0 || 0, "error : logical OR , both value is false.\n");

    assert(0, 0 || 1 && 0, "error : operater priority, logical OR and logical AND.\n");

    assert(5, 1 ? 5 : 0, "error : conditon expression(true pattern).\n");
    assert(5, 0 ? 3 : 5, "error : conditon expression(false pattern).\n");

    assert(8, 1 << 3 , "error : bitwise shift to left.\n");
    assert(1, 8 >> 3 , "error : bitwise shift to left.\n");

    assert(0, 255 >> 8, "error : bitwise shift to right 8bit.\n");

    b = 8;
    b <<= 3;
    assert(64, b, "error : bitwise left-shift and assign.\n");
    
    b >>= 3;
    assert(8, b, "error : bitwise right-shift and assign.\n");

    printf("expr test is completed !\n");
}
void test_enum(){

    enum ABC_ENUM{
        ENUM1 = 0,
        ENUM2,
        ENUM3 = 5, 
        ENUM4
    };

    enum ABC_ENUM a = ENUM3;

    assert(ENUM1, 0, "error : enum test1.\n");
    assert(ENUM2, 1, "error : enum test2.\n");
    assert(ENUM3, 5, "error : enum test3.\n");
    assert(ENUM4, 6, "error : enum test4.\n");
    assert(a, 5, "error : enum test.\n");

    printf("enum test is completed !\n");
}
void test_variable(){

    int a;
    int *b = &a;
    int array[5];
    int i = 0;
    char c = 10;

    int ifoo = foo(), jfoo = 4;
    assert(9, ifoo + jfoo, "error : variable initialize.\n");

    a = 3;
    assert(3, *b, "error : dereference.\n");
    *b = 5;
    assert(5, a, "error : using pointer\n");

    array[0] = 1;
    array[1] = 2;
    array[2] = 3;
    array[3] = 4;
    array[4] = 5;

    assert(1, *(array), "error : implicit cast of array type.\n");
    assert(2, *(array + 1), "error : implicit cast of array type.\n");
    assert(3, *(array + 2), "error : implicit cast of array type.\n");
    assert(4, *(array + 3), "error : implicit cast of array type.\n");
    assert(5, *(array + 4), "error : implicit cast of array type.\n");

    *(array) = 10;
    *(array + 1) = 20;
    *(array + 2) = 30;
    *(array + 3) = 40;
    *(array + 4) = 50;
    
    assert(10, *(array), "error : implicit cast of array type.\n");
    assert(20, *(array + 1), "error : implicit cast of array type.\n");
    assert(30, *(array + 2), "error : implicit cast of array type.\n");
    assert(40, *(array + 3), "error : implicit cast of array type.\n");
    assert(50, *(array + 4), "error : implicit cast of array type.\n");

    array[1 + 2] = 35;
    assert(35, array[3], "error : array index expr.\n");

    g_x = 5;
    assert(5, g_x, "error : grobal variable.\n");

    assert(25, c + 15, "error : char type.\n");

    char x = 2;
    char y = 8;
    assert(8, y, "error : char type : memory allocation.\n");
    assert(2, x, "error : char type : memory allocation.\n");

    int da[2][4];
    int k = 0;
    int i = 0;
    int j = 0;
    for(i = 0; i < 2; i++)
    for(j = 0; j < 4; j++)
        da[i][j] = k,k++;
    assert(3, da[0][3], "error : array of array access.\n");
    assert(6, da[1][2], "error : array of array access.\n");

    int dv[3][3][3];
    //memset(dv, 0, sizeof(dv));
    dv[0][1][2] = 25;
    dv[2][1][0] = 52;
    assert(25, dv[0][1][2], "error : array of array of array access.\n");
    assert(52, dv[2][1][0], "error : array of array of array access.2\n");

    short sa = 10;
    short sar[12];

    assert(10, sa, "error : short variable.\n");
    sar[sa] = 5;
    assert(5, sar[sa], "error : short array.\n");

    assert(0, test_static(), "error : test static variable.\n");
    assert(1, test_static(), "error : test static variable.\n");
    assert(2, test_static(), "error : test static variable.\n");

    printf("variable test is completed !\n");
}

void test_sizeof(){
    char c;
    int a;
    long x;
    int *b;
    
    char c_arr[5];
    int i_arr[5];
    long l_arr[5];

    assert(1, sizeof(c), "error : sizeof operater at char\n");
    assert(4, sizeof(a), "error : sizeof operater at int.\n");
    assert(8, sizeof(x), "error : sizeof operater at long.\n");
    assert(8, sizeof(b), "error : sizeof operater at pointer.\n");

    assert(5, sizeof(c_arr), "error : sizeof of array of char.\n");
    assert(20, sizeof(i_arr), "error : sizeof of array of int.\n");
    assert(40, sizeof(l_arr), "error : sizeof of array of long.\n");

    assert(1, sizeof(""), "error : sizeof operater at void string literal.\n");
    assert(5, sizeof("abcd"), "error : sizeof operater at string literal.\n");

    printf("sizeof test is completed !\n");
}

void test_string(){
    assert(0, ""[0], "error : void string literal.\n");
    assert(97, "abc"[0], "error : string literal access.\n");
    assert(98, "abc"[1], "error : string literal access.\n");
    assert(99, "abc"[2], "error : string literal access.\n");

    printf("string test is completed !\n");
}

void test_comment(){
    // exit_error("error : row comment.\n");

    /*
        exit_error("error : block comment.\n");
    */

    printf("comment test is completed !\n");
}

void test_literal(){
    assert(97, 'a', "error : character literal.\n");
    assert(65, 'A', "error : character literal.\n");
    assert(94, '^', "error : character literal.\n");

    printf("literal test is completed !\n");
}

void test_struct(){

    struct TEST_STRUCT {
        int a;
        long b;
        char ab[4];
        struct TEST_2STRUCT{
            long ai;
            long k;
        } abdd;
    };
    struct TEST_STRUCT k, abd, *c;
    
    abd.b = 25;
    abd.abdd.ai = 139;
    assert(25, abd.b, "error : struct member operater.\n");
    assert(139, abd.abdd.ai, "error : struct of strcut.\n");

    abd.ab[2] = 123;
    assert(123, abd.ab[2], "error : struct array member access.\n");

    long *x = &(abd.b);
    assert(25, *x, "error : struct member address.\n");

    c = &abd;
    c->abdd.k = 45;
    assert(25, c->b, "error : struct arrow operater.\n");
    assert(45, c->abdd.k, "error : arrow operater with struct of struct.\n");

    assert(32, sizeof(abd), "error : sizeof struct.\n");

    k.a = 124;
    assert(124, k.a, "error : named struct.\n");

    printf("struct test is completed !\n");    
}

int test_typedef(){

    typedef struct ABCD{
        int a;
        int b;
        char c;
    } ABC;

    typedef ABC AKC;
    AKC ak;
    ABC a, *b;
    a.a = 15;
    a.b = 12;
    a.c = 3;

    ak.a = 14;
    b = &a;

    assert(15, a.a, "error : typedef struct.\n");
    assert(12, a.b, "error : typedef struct.\n");
    assert(3, a.c, "error : typedef struct.\n");

    assert(15, b->a, "error : typedef struct pointer.\n");
    assert(12, b->b, "error : typedef struct pointer.\n");
    assert(3, b->c, "error : typedef struct pointer.\n");

    assert(14, ak.a, "error : typedef typedef.\n");


    printf("typedef test is completed.\n");    
}

int test_preprocess(){
    int a = TEST_DEF;
    assert(1, a, "error : #define num.\n");

#ifdef TEST_DEF1
    assert(0, 1, "error : not defined pattern , not invalid path.\n");
#else
    printf("#ifdef : not defined pattern #else path OK.\n");
#endif

#ifdef TEST_DEF
    printf("#ifdef : defined pattern #true path OK.\n");
#else
    assert(0, 1, "error : defined pattern , not invalid path.\n");
#endif

#ifdef TEST_DEF
#ifdef TEST_DEF1
    assert(0, 1, "error : Nesting #ifdef pattern 1.\n");
#else
    printf("#ifdef : Nesting pattern test OK.\n");
#endif
#else
    assert(0, 1, "error : Nesting #ifdef pattern 2.\n");
#endif

#ifdef TEST_DEF
    printf("#ifdef : No #else pattern test OK.\n");
#endif

#ifndef TEST_DEF1
    printf("#ifndef : No #else pattern test OK.\n");
#endif

#ifndef TEST_DEF
#ifdef TEST_DEF1
    1;
#endif
#else
#ifdef TEST_DEF
    printf("#ifndef : Nesting pattern test OK.\n");
#endif
#endif

    printf("preprocess test is completed.\n");
}

void test_cast(){

    struct TEST_DATA {
        char c;
        short s;
        int i;
        long l;
    };

    char c = 127;
    short s = 32767;
    int i = 2147483647;
    long l = 9223372036854775807;

    struct TEST_DATA test_str;

    // char data cast...
    assert((short)c, 127, "error : char to short cast.\n");
    assert((int)c, 127, "error : char to int cast.\n");
    assert((long)c, 127, "error : char to long cast.\n");

    // short data cast to...
    assert_c((char)s, -1, "error : short to char cast.\n");
    assert((int)s, 32767, "error : short to int cast.\n");
    assert((long)s, 32767, "error : short to long cast.\n");

    // int data cast to...
    assert_c((char)i, -1, "error : int to char cast.\n");
    assert_s((short)i, -1, "error : int to short cast.\n");
    assert((long)i, 2147483647, "error : int to long cast.\n");
    
    // long data cast to...
    assert_c((char)l, -1, "error : long to char cast.\n");
    assert_s((short)l, -1, "error : long to short cast.\n");
    assert((int)l, -1, "error : long to int cast.\n");

    test_str.c = 100;
    test_str.i = 100;
    test_str.l = 200;
    test_str.s = 1231;

    long* non_pointer = &test_str;

    assert(((struct TEST_DATA*)non_pointer)->s, 1231, "error : pointer cast.");

    printf("cast test is completed.\n");
}

int main(){   
    test_lib();
    test_expr();
    test_stmt();
    test_func();
    test_variable();
    test_sizeof();
    test_string();
    test_comment();
    test_struct();
    test_literal();
    test_enum();
    test_typedef();
    test_preprocess();
    test_cast();

    printf("test completed !\n");

    return 0;
}