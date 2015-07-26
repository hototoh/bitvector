CC=gcc
CFLAGS= -Wall -O3 -g -msse4.2 -std=gnu99

.PHONY: clean all
all: libbv test_bitvector

.SUFFIXES: .c .o
.c.o:
	$(CC) $(CFLAGS) -c $<

test_bitvector: test_bitvector.o libbv.a 
	$(CC) $(CFLAGS) $^ -o $@

libbv: bitvector.o
	$(AR) rcs $@.a $<

clean:
	rm -rf *.o *.a *~
