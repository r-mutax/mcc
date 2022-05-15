#!/bin/bash
assert() {
  expected="$1"
  input="$2"

  ./bin/mcc "$input" > ./test/tmp.s
  cc -static -o ./test/tmp ./test/tmp.s ./test/test_func/foo.o
  ./test/tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}
assert 42 "long main(){return 42;}"
assert 15 "long main(){return 3 + 16 -    4;}"
assert 14 "long main(){return 4+2*5;}"
assert 20 "long main(){return 25-10/2;}"
assert 35 "long main(){return (2+5) * 5;}"
assert 20 "long main(){return (-10 + 20) * 2;}"
assert 1 "long main(){return 5==5;}"
assert 0 "long main(){return 3==5;}"
assert 1 "long main(){return (5-3) != 6;}"
assert 0 "long main(){return 4 * 2 != 8;}"
assert 1 "long main(){return 5 > 4;}"
assert 0 "long main(){return 5 > 5;}"
assert 1 "long main(){return 4 >= 4;}"
assert 1 "long main(){return 5 >= 4;}"
assert 1 "long main(){return 4 < 5;}"
assert 1 "long main(){return 4 <= 4;}"
assert 38 "long main(){long abc; abc=15;long b; b=23; return abc+b;}"
assert 5 "long main(){return 5; 4*5;}"
assert 3 "long main(){long a; a = 0;while(a < 3) a = a + 1; return a;}"
assert 5 "long main(){long a; a = 1; if(a == 1) return 5; return 4;}"
assert 3 "long main(){long a; a=0; if(4 < 5) a = 2; else a = 3; return 3;}"
assert 6 "long main(){long a;a = 3; if(0 < 6) {a = 1; a = a + 5;} return a;}"
assert 4 "long main(){long a; long b;a = 0; b = 0; for(a = 1; a < 5; a = a + 1){b = b + 1;} return b;}"
assert 5 "long foo();long main(){return foo();}"
assert 15 "long add();long main(){return add(7, 8);}"
assert 5 "long footest(){return 5;} long main(){return footest();}"
assert 2 "long main(){return 12 % 5;}"
assert 3 "long main(){return 5 % 2 + 12 % 5;}"
assert 6 "long main(){ long a; long b;a = 5; b = &a; *b = 3; return a + *b; }"
assert 15 "long footest(long a, long b){return a+b;} long main(){return footest(3 , 12);}"
assert 5 "long main(){ long a; long *b; b = &a; *b = 5; return a;}"
assert 2 "long tmalloc();long main(){ long* a; tmalloc(&a); return *(a + 1);}"
assert 3 "long tmalloc();long main(){ long* a; tmalloc(&a); long* b; b = a + 3; return *(b - 1);}"
assert 8 "long main(){ long a; return sizeof (a) ;}"
assert 3 'long main(){long p[3]; p[0] = 3; p[1] = 2; long *q; q = p + 1; return p[0];}'
assert 123 'long main(){long a[10]; a[0] = 3; a[1] = 4; a[1+2*2] = 123; return a[5];}'
assert 4 'long main(){long a[10]; a[0] = 3; a[1] = 4; return a[1];}'
assert 4 'long x;long main(){long a[10]; *a = 3; *(a + 1) = 4; return *(a + 1);}'
assert 5 'long x; long main(){x = 5; return x;}'
assert 0 "long a;long main(){return 0;}"
assert 50 "long a[5]; long main(){ a[3] = 50; a[1] = 25; return a[3];}"
assert 81 "int main(){ int a; a = 67; int b; b = 14; return a + b;}"
assert 3 'int main() { int x[3]; *x=3; *(x+1)=4; *(x+2)=5; return *x; }'
assert 4 'int main() { int x[3]; *x=3; *(x+1)=4; *(x+2)=5; return *(x+1); }'
assert 5 'int main() { int x[3]; *x=3; *(x+1)=4; *(x+2)=5; return *(x+2); }'
assert 3 'int main() { int x[3]; *x=3; x[1]=4; x[2]=5; return *x; }'
assert 4 'int main() { int x[3]; *x=3; x[1]=4; x[2]=5; return *(x+1); }'
assert 5 'int main() { int x[3]; *x=3; x[1]=4; x[2]=5; return *(x+2); }'
assert 5 'int main() { int x[3]; *x=3; x[1]=4; x[2]=5; return *(x+2); }'
assert 4 'int main() { int a; return sizeof(a);}'
assert 12 'int main() { int a[3]; return sizeof(a);}'
assert 24 'int main() { long a[3]; return sizeof(a);}'
assert 9 'int intadd(int a, int b){ return a + b;} int main(){return intadd(4, 5);}'
assert 25 'int main(){ char a; a = 10; return 15 + a;}'
assert 1 'int main() { char x; x=1; return x; }'
assert 1 'int main() { char x; x=1; char y; y=2; return x; }'
assert 2 'int main() { char x; x=1; char y; y=2; return y; }'
assert 1 'int main() { char x; return sizeof(x); }'
assert 10 'int main() { char x[10]; return sizeof(x); }'
assert 1 'int sub_char(char a, char b, char c) { return a-b-c; } int main() { return sub_char(7, 3, 3); } '
echo OK