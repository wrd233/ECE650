#include "my_malloc.h"
#include <stdint.h>
#include <unistd.h>
#include <assert.h>

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

static void print_free_list(){
    if(block_head == NULL){
        printf("free list中尚未有元素");
        return;
    }
    printf("~~~~~~~~~~~~~~~~~~\n");
    block_t* ptr = free_head;
    while(ptr->free_next != free_head){
        printf("payload_size = %lu, is_allocated = %d\n",ptr->payload_size, ptr->is_allocated);
         ptr = ptr->free_next;
    }
    printf("payload_size = %lu, is_allocated = %d\n",ptr->payload_size, ptr->is_allocated);
    printf("~~~~~~~~~~~~~~~~~~\n");
}

// TODO: 使用first fit的策略找到合适的block
static block_t* find_fit_ff(size_t payload_size){
    return NULL;
}

// TODO: 使用best fit的策略找到合适的block
static block_t* find_fit_bf(size_t payload_size){
    return NULL;
}


static void drop_from_free_list(block_t* block){
    assert(block!=NULL);
    assert(block->free_next!=NULL && block->free_prev!=NULL);

    // 将当前节点从双向链表中去除
    if(block->free_next == block){  // 此时free list中只有一个元素
        free_head = NULL;
        block->free_next = NULL;
        block->free_prev = NULL;
    }else{
        if (free_head == block) {
            free_head = block->free_next;
        }
        block->free_prev->free_next = block->free_next;
        block->free_next->free_prev = block->free_prev;
    }

    // print_free_list();
}


static void add_to_free_list(block_t* block){
    assert(block != NULL);
    assert(block->free_prev==NULL && block->free_next == NULL);

    if(free_head != NULL){
        block->free_next = free_head;
        block->free_prev = free_head->free_prev;
        free_head->free_prev = block;
        block->free_prev->free_next = block;
    }else{
        block->free_next = block;
        block->free_prev = block;
    }

    free_head = block;
    // print_free_list();
}

// 如果原先的内存池不够了，那么就拓展制定大小的block
static block_t* extend_heap(size_t payload_size){
    printf("调用了extend\n");
    size_t size_sum = sizeof(struct block) + payload_size;
    block_t* new_ptr = sbrk(size_sum);

    new_ptr->payload_size = payload_size;
    new_ptr->is_allocated = 1;
    new_ptr->prev = block_tail;
    new_ptr->next = NULL;
    new_ptr->free_next = NULL;
    new_ptr->free_prev = NULL;

    if(block_tail != NULL){
        block_tail->next = new_ptr;
    }
    block_tail = new_ptr;
    if(block_head == NULL){
        block_head = new_ptr;
    }

    print_list(block_head);

    return new_ptr;
}

// 收回内存的时候直接合并相邻的; 把新的空闲块加入到空闲链表当中(头插吧要不)
static void block_free(block_t* block){
    printf("调用了free\n");
    if(free_head == NULL){
        free_head = block;
    }

    // 首先将当前block的标志位清除
    block->is_allocated = 0;

    block_t* curr = block;
    assert(curr->free_next == NULL && curr->free_next == NULL);
    // 尝试与相邻的块合并
    block_t* prev_block = block->prev;
    if(prev_block != NULL && prev_block->is_allocated == 0){
        prev_block->next = curr->next;
        prev_block->payload_size += sizeof(block_t) + curr->payload_size;
        curr = prev_block;
    }

    block_t* next_block = block->next;
    if(next_block != NULL && next_block->is_allocated == 0){
        curr->next = next_block->next;
        // 首先将next_block从free list中删除
        drop_from_free_list(next_block);
        curr->payload_size += sizeof(block_t) + next_block->payload_size;
    }

    if(curr->free_next == NULL){
        add_to_free_list(curr);
    }

    print_free_list();
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
    block_free(ptr);
}


void bf_free(void * ptr){
    block_free(ptr);
}

unsigned long get_largest_free_data_segment_size() {
  return largest_free_size;
}

unsigned long get_total_free_size() {
  return free_size_sum;
}