#!/bin/bash
assert() {
  expected="$1"
  input="$2"

  ./bin/mcc "$input" > ./test/tmp.s
  cc -o ./test/tmp ./test/tmp.s ./test/test_func/foo.o
  ./test/tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

assert 0 "main(){return 0;}"
assert 42 "main(){return 42;}"
assert 15 "main(){return 3 + 16 -    4;}"
assert 14 "main(){return 4+2*5;}"
assert 20 "main(){return 25-10/2;}"
assert 35 "main(){return (2+5) * 5;}"
assert 20 "main(){return (-10 + 20) * 2;}"
assert 1 "main(){return 5==5;}"
assert 0 "main(){return 3==5;}"
assert 1 "main(){return (5-3) != 6;}"
assert 0 "main(){return 4 * 2 != 8;}"
assert 1 "main(){return 5 > 4;}"
assert 0 "main(){return 5 > 5;}"
assert 1 "main(){return 4 >= 4;}"
assert 1 "main(){return 5 >= 4;}"
assert 1 "main(){return 4 < 5;}"
assert 1 "main(){return 4 <= 4;}"
assert 38 "main(){abc=15;b=23; return abc+b;}"
assert 5 "main(){return 5; 4*5;}"
assert 3 "main(){a = 0;while(a < 3) a = a + 1; return a;}"
assert 5 "main(){a = 1; if(a == 1) return 5; return 4;}"
assert 3 "main(){a=0; if(4 < 5) a = 2; else a = 3; return 3;}"
assert 6 "main(){a = 3; if(0 < 6) {a = 1; a = a + 5;} return a;}"
assert 4 "main(){a = 0; b = 0; for(a = 1; a < 5; a = a + 1){b = b + 1;} return b;}"
assert 5 "main(){return foo();}"
assert 15 "main(){return add(7, 8);}"
assert 5 "foo(){return 5;} main(){return foo();}"
assert 2 "main(){return 12 % 5;}"
assert 3 "main(){return 5 % 2 + 12 % 5;}"
assert 6 "main(){ a = 5; b = &a; *b = 3; return a + *b; }"
echo OK