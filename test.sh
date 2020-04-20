#!/bin/bash
try()  {
    expected="$1"
    input="$2"

    ./9cc "$input" > tmp.s
    gcc -o tmp tmp.s
    ./tmp
    actual="$?"

    if [ "$actual" = "$expected" ]; then
        echo "$input => $actual"
    else
        echo "$input => $expected expected, but got $actual"
        exit 1
    fi
}

try 0 '0;'
try 42 '42;'
try 21 '5+20-4;'
try 41 '12 + 34 -5 ;'
try 47 '5+6*7;'
try 15 '5*(9-6);'
try 4 '(3+5)/2;'
try 3 '5+ -(1+1);'
try 1 '2==2;'
try 0 '4==5;'
try 0 '2!=2;'
try 1 '4!=5;'
try 1 '3>2;'
try 0 '2>3;'
try 1 '3>=3;'
try 0 '2>=3;'
try 0 '3<2;'
try 1 '2<3;'
try 0 '5<=3;'
try 1 '2<=3;'
try 0 '2<=(3+2)*-2;'
try 1 '2<=(3+2)*2;'
try 3 'a = 3;'
try 22 'b = 5 * 6 - 8;'
try 14 '3 + 22/2;'
try 14 'a = 3;_ = 5 * 6 -8; a + _ /2;'
try 14 'aa3_a = 3;_ = 5 * 6 -8; aa3_a + _ /2;'
try 5  'return 5;'
try 3  'aa3_a = 3;return aa3_a;'

IF_TEST=`cat test_asset/if.c`
try 3   "$IF_TEST"
WHILE_TEST=`cat test_asset/while.c`
try 5  "$WHILE_TEST"
FOR_TEST=`cat test_asset/for.c`
try 15  "$FOR_TEST"
echo ok