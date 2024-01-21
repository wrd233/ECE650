#ifndef MY_MALLOC_H
#define MY_MALLOC_H
#include <stdio.h>
#include <stdlib.h>

//First Fit malloc
void * ff_malloc(size_t size);

//Best Fit malloc
void * bf_malloc(size_t size);

//First Fit free
void ff_free(void * ptr);

//Best Fit free
void bf_free(void * ptr);

#endif /* MY_MALLOC_H */