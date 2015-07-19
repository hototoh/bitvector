CC=gcc
CFLAGS= -Wall -O3 -msse4.2 -std=c99

.PHONY: clean all
all: libbv

.SUFFIXES: .c .o
.c.o:
	$(CC) $(CFLAGS) -c $<

libbv: bitvector.o
	$(AR) rcs $@.a $<

clean:
	rm -rf *.o *.a *~
