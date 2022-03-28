CFLAGS=-I ../include -g
CC=gcc $(CFLAGS)

runtests: tests
	./tests

tests: tests.o ../libfsdb.a ../include/fsdb.h
	$(CC) -L.. -o $@ $< -lfsdb

%.o: %.c
	$(CC) -c -o $@ $<
