int exit_error(char p);
int printf(char* p);
int foo();
int print_int(int a);

long g_x;

int assert(int lhs, int rhs, char* p){
    if(lhs != rhs)
        exit_error(p);
    return 0;
}

int assert2(int lhs, int rhs, char* p){
    print_int(rhs);
    print_int(lhs);
    exit_error(p);
    return 0;
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

int test_func(){

    assert(5, test_add(1, 4), "error : multi paramaters function call.\n");

    
    printf("function test is completed !\n");
}

int test_stmt(){

    assert(5, test_return(), "error : return.\n");

    int a = 0;
    int i = 0;
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

    printf("stmt test is completed !\n");
}

int test_expr(){

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

    printf("expr test is completed !\n");
}

int test_variable(){

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


    printf("variable test is completed !\n");
}

int test_sizeof(){
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

int test_string(){
    assert(0, ""[0], "error : void string literal.\n");
    assert(97, "abc"[0], "error : string literal access.\n");
    assert(98, "abc"[1], "error : string literal access.\n");
    assert(99, "abc"[2], "error : string literal access.\n");

    printf("string test is completed !\n");
}

int main(){   
    test_expr();
    test_stmt();
    test_func();
    test_variable();
    test_sizeof();
    test_string();

    printf("test completed !\n");

    return 0;
}