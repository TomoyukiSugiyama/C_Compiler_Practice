CFLAGS=-std=c11 -g -static

9cc: sample.c
		gcc -o 9cc sample.c
		
test: 9cc
		./test.sh

clean:
		rm -f 9cc *.o *~ tmp*

.PHONY: test clean
