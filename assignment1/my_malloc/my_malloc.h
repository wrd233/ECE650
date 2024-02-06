#ifndef MY_MALLOC_H
#define MY_MALLOC_H
#include <stdio.h>
#include <stdlib.h>
#include "macro.h"

//First Fit malloc
void * ff_malloc(size_t size);

//Best Fit malloc
void * bf_malloc(size_t size);

//First Fit free
void ff_free(void * ptr);

//Best Fit free
void bf_free(void * ptr);

unsigned long get_largest_free_data_segment_size(); 
unsigned long get_total_free_size();


typedef struct block block_t;
struct block
{
    size_t payload_size; 
    int is_allocated;        
    block_t* prev;  
    block_t* next;              // TODO: 感觉可以通过payload计算出来，但可能因为对齐之类的原因，有点难算；之后优化               
    block_t* free_next;         // free block中的next
    block_t* free_prev;         // free block中的prev
    // unsigned char payload[0];   // 真正提供出去的地址
};

static block_t* block_head = NULL;
static block_t* block_tail = NULL;
static block_t* free_head = NULL;

void print_free_list();
void drop_from_free_list(block_t* block);
void add_to_free_list(block_t* block);
void block_free(block_t* block);
block_t* extend_heap(size_t payload_size);

#endif /* MY_MALLOC_H */