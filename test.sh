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

try 0 'main(){0;}'
try 42 'main(){42;}'
try 21 'main(){5+20-4;}'
try 41 'main(){12 + 34 -5 ;}'
try 47 'main(){5+6*7;}'
try 15 'main(){5*(9-6);}'
try 4 'main(){(3+5)/2;}'
try 3 'main(){5+ -(1+1);}'
try 1 'main(){2==2;}'
try 0 'main(){4==5;}'
try 0 'main(){2!=2;}'
try 1 'main(){4!=5;}'
try 1 'main(){3>2;}'
try 0 'main(){2>3;}'
try 1 'main(){3>=3;}'
try 0 'main(){2>=3;}'
try 0 'main(){3<2;}'
try 1 'main(){2<3;}'
try 0 'main(){5<=3;}'
try 1 'main(){2<=3;}'
try 0 'main(){2<=(3+2)*-2;}'
try 1 'main(){2<=(3+2)*2;}'
try 3 'main(){a = 3;}'
try 22 'main(){b = 5 * 6 - 8;}'
try 14 'main(){3 + 22/2;}'
try 14 'main(){a = 3;_ = 5 * 6 -8; a + _ /2;}'
try 14 'main(){aa3_a = 3;_ = 5 * 6 -8; aa3_a + _ /2;}'
try 5  'main(){return 5;}'
try 3  'main(){aa3_a = 3;return aa3_a;}'

IF_TEST=`cat test_asset/if.c`
try 3   "$IF_TEST"
WHILE_TEST=`cat test_asset/while.c`
try 5  "$WHILE_TEST"
FOR_TEST=`cat test_asset/for.c`
try 15  "$FOR_TEST"

func_call()  {
    input="$1"
    lib="$2"

    ./9cc "$input" > tmp.s
    gcc -o tmp tmp.s $lib
    ./tmp
}
(cd test_asset/lib ; make)

FUNC_TEST=`cat test_asset/func_call_no_arg.c`
FUNC_LIB="test_asset/lib/func_call_no_arg_lib.o"
func_call "$FUNC_TEST" $FUNC_LIB

FUNC_TEST=`cat test_asset/func_call_01_arg.c`
FUNC_LIB="test_asset/lib/func_call_01_arg_lib.o"
func_call "$FUNC_TEST" $FUNC_LIB

FUNC_TEST=`cat test_asset/func_call_02_arg.c`
FUNC_LIB="test_asset/lib/func_call_02_arg_lib.o"
func_call "$FUNC_TEST" $FUNC_LIB

FUNC_TEST=`cat test_asset/func_call_03_arg.c`
FUNC_LIB="test_asset/lib/func_call_03_arg_lib.o"
func_call "$FUNC_TEST" $FUNC_LIB

FUNC_TEST=`cat test_asset/func_call_04_arg.c`
FUNC_LIB="test_asset/lib/func_call_04_arg_lib.o"
func_call "$FUNC_TEST" $FUNC_LIB

FUNC_TEST=`cat test_asset/func_call_05_arg.c`
FUNC_LIB="test_asset/lib/func_call_05_arg_lib.o"
func_call "$FUNC_TEST" $FUNC_LIB

FUNC_TEST=`cat test_asset/func_call_06_arg.c`
FUNC_LIB="test_asset/lib/func_call_06_arg_lib.o"
func_call "$FUNC_TEST" $FUNC_LIB

try 5  'main(x,y){a=5;x=3;y=2;return a;}'
try 3  'main(x,y){a=5;x=3;y=2;return x;}'
try 2  'main(x,y){a=5;x=3;y=2;return y;}'

#try 4  'main(){a=5;x=3;y=2;foo(x,y);return y;} foo(x,y){return y+2;}'

echo ok