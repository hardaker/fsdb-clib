CFLAGS=-I ../include
CC=gcc $(CFLAGS)

runtests: tests
	./tests

tests: tests.o
	$(CC) -L.. -o $@ $< -lfsdb

%.o: %.c
	$(CC) -c -o $@ $<
