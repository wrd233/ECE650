#include "my_malloc.h"
#include <stdint.h>
#include <unistd.h>

typedef struct block block_t;
struct block
{
    // TODO: 如果要考虑对其，这里跨平台的不统一可能导致无法对齐
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
unsigned long largest_free_size = 0;    // TODO: 添加这个的全局逻辑
unsigned long free_size_sum = 0;        // TODO: 添加这个的全局逻辑

static block_t* get_next_block(block_t* curr){
    // if(curr == block_tail){
    //     return NULL;
    // }
    // size_t offset = curr->payload_size + sizeof(struct block);
    // return (block_t*)((unsigned char*)curr + offset);
    return curr->next;
}

static void print_list(){
    if(block_head == NULL){
        printf("链表中尚未有元素");
        return;
    }
    printf("=================\n");
    for(block_t* ptr = block_head; ptr != NULL; ptr = get_next_block(ptr)){
        printf("payload_size = %lu, is_allocated = %d\n",ptr->payload_size, ptr->is_allocated);
    }
    printf("=================\n");
}

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
    new_ptr->prev = block_tail;

    if(block_tail != NULL){
        block_tail->next = new_ptr;
    }
    block_tail = new_ptr;
    if(block_head == NULL){
        block_head = new_ptr;
    }

    // print_list(block_head);

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