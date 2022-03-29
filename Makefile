.PHONY:

CFLAGS=-I include -g -Wall
CC=gcc $(CFLAGS)

OBJS=fsdb.o

testit: libfsdb.a
	make -C tests

libfsdb.a: Makefile $(OBJS)
	rm -f $@
	ar cr $@ $(OBJS)

%.o: %.c include/fsdb.h
	$(CC) -c -o $@ $<
