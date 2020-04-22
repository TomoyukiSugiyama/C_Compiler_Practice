CFLAGS=-std=c11 -g -static -w
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

libs: $(OBJS)
	echo "make libs"

clean:
		rm -f libs *.o *~ tmp*

.PHONY: clean