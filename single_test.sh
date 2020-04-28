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

func_call()  {
    input="$1"
    lib="$2"

    ./9cc "$input" > tmp.s
    gcc -o tmp tmp.s $lib
    echo "$input =>"
    ./tmp
}
(cd test_asset/lib ; make)

FIBONACCI_TEST=`cat test_asset/fibonacci.c`
try 55 "$FIBONACCI_TEST"

