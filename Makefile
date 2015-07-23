CC=gcc
CFLAGS= -Wall -O3 -msse4.2 -std=gnu99

.PHONY: clean all
all: libbv test_bitvector

.SUFFIXES: .c .o
.c.o:
	$(CC) $(CFLAGS) -c $<

test_bitvector: libbv.a test_bitvector.o
	$(CC) $(CFLAGS) $^ -o $@

libbv: bitvector.o
	$(AR) rcs $@.a $<

clean:
	rm -rf *.o *.a *~
