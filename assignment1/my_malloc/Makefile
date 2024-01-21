CC = gcc
CFLAGS = -Wall -Werror -fPIC

all: libmymalloc.so

libmymalloc.so: my_malloc.o
	$(CC) -shared -o libmymalloc.so my_malloc.o

my_malloc.o: my_malloc.c my_malloc.h
	$(CC) $(CFLAGS) -c my_malloc.c

clean:
	rm -f libmymalloc.so my_malloc.o