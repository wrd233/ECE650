# CC = gcc
# CFLAGS = -fPIC
# DEBUG_CFLAGS = -DDEBUG

# all: libmymalloc.so

# libmymalloc.so: my_malloc.o
# 	$(CC) -shared -o libmymalloc.so my_malloc.o

# my_malloc.o: my_malloc.c my_malloc.h
# 	$(CC) $(CFLAGS) -c my_malloc.c

# debug: CFLAGS += $(DEBUG_CFLAGS)
# debug: clean all

# clean:
# 	rm -f libmymalloc.so my_malloc.o

CC = gcc
CFLAGS = -fPIC
DEBUG_CFLAGS = -DDEBUG

all: libmymalloc.so

libmymalloc.so: my_malloc.o
	$(CC) -shared -o libmymalloc.so my_malloc.o

my_malloc.o: my_malloc.c my_malloc.h
	$(CC) $(CFLAGS) -c my_malloc.c

debug: CFLAGS += $(DEBUG_CFLAGS)
debug: clean a.out

a.out: main.o my_malloc.o
	$(CC) -o a.out main.o my_malloc.o

main.o: main.c my_malloc.h
	$(CC) $(CFLAGS) -c main.c

clean:
	rm -f libmymalloc.so a.out main.o my_malloc.o