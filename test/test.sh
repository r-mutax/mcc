#!/bin/bash
assert() {
  expected="$1"
  input="$2"

  ./bin/mcc "$input" > ./test/tmp.s
  cc -o ./test/tmp ./test/tmp.s
  ./test/tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

assert 0 "0;"
assert 42 "42;"
assert 15 "3 + 16 -    4;"
assert 14 "4+2*5;"
assert 20 "25-10/2;"
assert 35 "(2+5) * 5;"
assert 20 "(-10 + 20) * 2;"
assert 1 "5==5;"
assert 0 "3==5;"
assert 1 "(5-3) != 6;"
assert 0 "4 * 2 != 8;"
assert 1 "5 > 4;"
assert 0 "5 > 5;"
assert 1 "4 >= 4;"
assert 1 "5 >= 4;"
assert 1 "4 < 5;"
assert 1 "4 <= 4;"
assert 38 "abc=15;b=23;abc+b;"
echo OK