#ifndef MY_MALLOC_H
#define MY_MALLOC_H
#include <stdio.h>
#include <stdlib.h>
#include "macro.h"


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

typedef struct global_list_info {
    block_t** block_head_ptr;
    block_t** block_tail_ptr;
    block_t** free_head_ptr;
} global_t;

//Best Fit malloc
void * bf_malloc(size_t size, global_t* global_list_info, int is_sbrk_locked);

//Best Fit free
void bf_free(void * ptr, global_t* global_list_info);

//Thread Safe malloc/free: locking version
void *ts_malloc_lock(size_t size);
void ts_free_lock(void *ptr);

//Thread Safe malloc/free: non-locking version
void *ts_malloc_nolock(size_t size);
void ts_free_nolock(void *ptr);

unsigned long get_largest_free_data_segment_size(); 
unsigned long get_total_free_size();

void print_free_list(global_t* global_list_info);
void drop_from_free_list(block_t* block, global_t* global_list_info);
void add_to_free_list(block_t* block, global_t* global_list_info);
void block_free(block_t* block, global_t* global_list_info);
block_t* extend_heap(size_t payload_size, global_t* global_list_info, int is_sbrk_locked);

#endif /* MY_MALLOC_H */