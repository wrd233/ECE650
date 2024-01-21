#include "my_malloc.h"
#include <stdint.h>
#include <unistd.h>

typedef struct block block_t;
struct block
{
    // TODO: 如果要考虑对其，这里跨平台的不统一可能导致无法对齐
    size_t payload_size; 
    int is_allocated;           
    block_t* next; 
    block_t* prev;
    unsigned char payload[0];   // 真正提供出去的地址
};

static block_t* block_head = NULL;
static block_t* free_head = NULL;
unsigned long largest_free_size = 0;    // TODO: 添加这个的全局逻辑
unsigned long free_size_sum = 0;        // TODO: 添加这个的全局逻辑

// TODO: 使用first fit的策略找到合适的block
static block_t* find_fit_ff(size_t payload_size){
    return NULL;
}

// TODO: 使用best fit的策略找到合适的block
static block_t* find_fit_bf(size_t payload_size){
    return NULL;
}

// 如果原先的内存池不够了，那么就拓展制定大小的block
static block_t* extend_heap(size_t payload_size){
    size_t size_sum = sizeof(struct block) + payload_size;
    block_t* new_ptr = sbrk(size_sum);

    new_ptr->payload_size = payload_size;
    new_ptr->is_allocated = 1;

    if(block_head == NULL){
        block_head = new_ptr;
    }

    return new_ptr;
}

// TODO: 收回内存的时候直接合并相邻的
// TODO: 把新的空闲块加入到空闲链表当中(头插吧要不)
static void block_free(block_t* block){
    if(free_head == NULL){
        free_head = block;
    }
}


void * ff_malloc(size_t size){
    // 首先尝试从空闲链表中的空闲块中分配
    block_t* block_ptr = find_fit_ff(size);

    if(block_ptr == NULL){  // 此时空闲链表中没有合适的块，所以需要进行拓展
        block_ptr = extend_heap(size);
    }else{  // 从空闲链表中找到合适的块
        // TODO: 从空闲链表中删除这一空闲块
    }
    return block_ptr;
}

void * bf_malloc(size_t size){
    // 首先尝试从空闲链表中的空闲块中分配
    block_t* block_ptr = find_fit_bf(size);

    if(block_ptr == NULL){  // 此时空闲链表中没有合适的块，所以需要进行拓展
        block_ptr = extend_heap(size);
    }else{  // 从空闲链表中找到合适的块
        // TODO: 从空闲链表中删除这一空闲块
    }
    return block_ptr;
}


void ff_free(void * ptr){
    block_free(NULL);
}


void bf_free(void * ptr){
    block_free(NULL);
}

unsigned long get_largest_free_data_segment_size() {
  return largest_free_size;
}

unsigned long get_total_free_size() {
  return free_size_sum;
}