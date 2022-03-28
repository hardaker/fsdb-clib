.PHONY:

CFLAGS=-I include -g
CC=gcc $(CFLAGS)

OBJS=fsdb.o

testit: libfsdb.a
	make -C tests

libfsdb.a: Makefile $(OBJS)
	rm -f $@
	ar cr $@ $(OBJS)

%.o: %.c
	$(CC) -c -o $@ $<
