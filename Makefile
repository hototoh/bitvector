CC=gcc
CFLAGS= -Wall -O3 -g -msse4.2 -mavx -mavx2 -std=gnu99

OBJS = benchmark.o bitvector.o

.PHONY: clean all
all: libbv benchmark test_bitvector
#all: libbv test_bitvector

.SUFFIXES: .c .o
.c.o:
	$(CC) $(CFLAGS) -c $<

test_bitvector: test_bitvector.o libbv.a 
	$(CC) $(CFLAGS) $^ -o $@

libbv: bitvector.o
	$(AR) rcs $@.a $<

benchmark: $(OBJS)
	$(CC) $(CFLAGS) $^ -o benchmark

clean:
	rm -rf *.o *.a *~
