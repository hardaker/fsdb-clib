.PHONY:

OBJS=fsdb.o

testit: libfsdb.a
	make -C tests

libfsdb.a: Makefile $(OBJS)
	ld -static -o $@ $(OBJS)

%.o: %.c
	$(CC) -I include -c -o $@ $<
