#include "my_malloc.h"
#include <stdint.h>

typedef struct block block_t;
struct block
{
    uint64_t header;            // block size + 状态 TODO: 这里的64位是为了支持64位机器，但是到32位机器上就可能浪费，而且这里的不统一可能导致无法对齐
    block_t* next; 
    block_t* prev;
    unsigned char payload[0];   // 真正提供出去的地址
};


size_t get_size(block_t* block_ptr){
    // 提取高 61 位
    size_t size = (size_t)(block_ptr->header >> 3);
    return size;
}

int get_state(block_t* block_ptr){
    return (block_ptr->header & 1) ? 1 : 0
}


static block_t* block_head = NULL;
static block_t* free_head = NULL;

// 使用first fit的策略找到合适的block
block_t* find_fit_ff(){

}

// 使用best fit的策略找到合适的block
block_t* find_fit_bf(){

}





void * ff_malloc(size_t size){
    // 1.
    return NULL;
}

void * bf_malloc(size_t size){
    return NULL;
}


void ff_free(void * ptr){

}


void bf_free(void * ptr){

}