#include "my_malloc.h"
#include <stdint.h>
#include <unistd.h>
#include <assert.h>

unsigned long largest_free_size = 0;    // TODO: 添加这个的全局逻辑
unsigned long free_size_sum = 0;        // TODO: 添加这个的全局逻辑

// TODO:
static void calFree(){
    // if(free_head == NULL){
    //     return;
    // }
    // block_t* ptr = free_head;
    // free_size_sum = 0;
    // largest_free_size = 0;
    // while(ptr->free_next != free_head){
    //     free_size_sum += ptr->payload_size;
    //     largest_free_size = largest_free_size > ptr->payload_size ? largest_free_size : ptr->payload_size;
    //     ptr = ptr->free_next;
    // }
    // free_size_sum += ptr->payload_size;
    // largest_free_size = largest_free_size > ptr->payload_size ? largest_free_size : ptr->payload_size;

}

static block_t* get_next_block(block_t* curr){
    // if(curr == block_tail){
    //     return NULL;
    // }
    // size_t offset = curr->payload_size + sizeof(struct block);
    // return (block_t*)((unsigned char*)curr + offset);
    return curr->next;
}

void print_list(){
    if(block_head == NULL){
        INFO("list中尚未有元素\n");
        return;
    }
    INFO("                     (list):");
    block_t* ptr = block_head;
    while(ptr != NULL){
        INFO("%lu/%d->",ptr->payload_size, ptr->is_allocated);
        ptr = ptr->next;
    }
    INFO("\n");
}

void print_free_list(){
    if(free_head == NULL){
        INFO("free list中尚未有元素\n");
        return;
    }
    INFO("                     (free_list):");
    block_t* ptr = free_head;
    while(ptr != NULL){
        INFO("%lu/%d->",ptr->payload_size, ptr->is_allocated);
        ptr = ptr->free_next;
    }
    INFO("\n");
}

// 使用first fit的策略找到合适的block
static block_t* find_fit_ff(size_t payload_size){
    INFO("尝试寻找ff，当前寻找的size为%lu\n",payload_size);
    block_t* current = free_head;

    for(block_t* curr = free_head; curr != NULL; curr = curr->free_next){
        if(curr->payload_size >= payload_size){
            return curr;
        }
    }

    return NULL;
}

// 使用best fit的策略找到合适的block
static block_t* find_fit_bf(size_t payload_size){
    INFO("尝试寻找bf，当前寻找的size为%lu\n",payload_size);
    block_t* best_fit_ptr = NULL;

    for(block_t* curr = free_head; curr != NULL; curr = curr->free_next){
        // 判断该节点能否承载payload
        if(curr->payload_size >= payload_size){
            if(best_fit_ptr == NULL){
                best_fit_ptr = curr;
            }else{
                best_fit_ptr = best_fit_ptr->payload_size < curr->payload_size ? best_fit_ptr : curr;
            }
        }
    }

    return best_fit_ptr;
}

/*
 *  注：从空闲链表中删除不意味着is_allocated需要置为1
*/
void drop_from_free_list(block_t* block){
    INFO("drop_from_free_list()\n");
    assert(block!=NULL);
    assert(block->is_allocated == 0);
    assert(free_head != NULL);

    // 将当前节点从双向链表中去除
    if(free_head == block && block->free_next == NULL){  // 此时free list中只有一个元素
        assert(block->free_prev == NULL);
        free_head = NULL;
    }else{
        assert(block->free_next!=NULL || block->free_prev!=NULL);
        if (free_head == block) {
            free_head = block->free_next;
        }
        if(block->free_prev != NULL){
            block->free_prev->free_next = block->free_next;
        }
        if(block->free_next != NULL){
            block->free_next->free_prev = block->free_prev;
        }
    }
    block->free_next = NULL;
    block->free_prev = NULL;
    print_free_list();
}

void add_to_free_list(block_t* block){
    INFO("add_to_free_list()\n");
    assert(block != NULL);
    assert(block->free_prev==NULL && block->free_next == NULL);

    if(block->is_allocated == 1){
        block->is_allocated = 0;
        INFO("[warning] 添加到free_list中的block的is_allocated标志位为1\n");
    }

    if(free_head != NULL){
        block->free_next = free_head;
        block->free_prev = NULL;
        free_head->free_prev = block;
    }else{
        block->free_next = NULL;
        block->free_prev = NULL;
    }

    free_head = block;
    print_free_list();
}

/*
 * 
*/
void drop_from_list(block_t* block){
    assert(block!=NULL);
    assert(block_head != block);    // 理论上是不会删除第一个块的，所以该函数内操作的block都有前序block
    assert(block->prev != NULL);
    assert(block_head != block || block_tail != block); //不可能将唯一的块删除

    if(block_tail == block){
        block_tail = block->prev;
    }

    block->prev->next = block->next;
    if(block->next != NULL){
        block->next->prev = block->prev;
    }

    block->next = NULL;
    block->prev = NULL;
}

/*
 * 
*/
void add_to_list_tail(block_t* block){
    if(block_tail != NULL){
        block_tail->next = block;
    }
    block_tail = block;

    if(block_head == NULL){
        block_head = block;
    }

    print_list();
}

// 如果原先的内存池不够了，那么就拓展制定大小的block
block_t* extend_heap(size_t payload_size){
    // printf("调用了extend\n");
    size_t size_sum = sizeof(struct block) + payload_size;
    block_t* new_ptr = sbrk(size_sum);

    new_ptr->payload_size = payload_size;
    new_ptr->is_allocated = 1;
    new_ptr->prev = block_tail;
    new_ptr->next = NULL;
    new_ptr->free_next = NULL;
    new_ptr->free_prev = NULL;

    add_to_list_tail(new_ptr);

    return new_ptr;
}

// 收回内存的时候直接合并相邻的; 把新的空闲块加入到空闲链表当中(头插吧要不)
void block_free(block_t* block){
    assert(block != NULL);
    assert(block->is_allocated == 1);
    assert(block->free_prev == NULL && block->free_next == NULL);

    // 首先将当前block的标志位清除
    block->is_allocated = 0;

    if(free_head == NULL){
        free_head = block;
    }else{
        block_t* merged_block_ptr = block;
        block_t* prev_block = block->prev;
        block_t* next_block = block->next;
        // 尝试与prev的块合并
        if(prev_block != NULL && prev_block->is_allocated == 0){
            INFO("prev为free block, 尝试合并\n");
            drop_from_free_list(prev_block);
            drop_from_list(block);
            prev_block->payload_size += sizeof(block_t) + block->payload_size;
            merged_block_ptr = prev_block;
        }

        // 尝试与next的块合并
        if(next_block != NULL && next_block->is_allocated == 0){
            INFO("next为free block, 尝试合并\n");
            drop_from_free_list(next_block);
            drop_from_list(next_block);
            merged_block_ptr->payload_size += sizeof(block_t) + next_block->payload_size;
        }

        add_to_free_list(merged_block_ptr);
    }

    print_free_list();
    print_list();
}

// TODO: 将blcok分割成payload_size和剩下的部分，分割失败返回NULL
static block_t* splitBlock(block_t* block, size_t payload_size){
    // TODO: 分割空闲块，并将分割之后的后半个block加入到空闲链表当中
    assert(block != NULL);
    // printf("当前块的大小为%lu, 期望分配%lu\n",block->payload_size, payload_size);

    if(block->payload_size >= sizeof(block_t) + payload_size){
        // 创建新的节点
        size_t offset = payload_size + sizeof(block_t);
        block_t* new_block_ptr = (block_t*)((unsigned char*)block + offset);
        new_block_ptr->free_prev = NULL;
        new_block_ptr->free_next = NULL;
        new_block_ptr->is_allocated = 0;
        new_block_ptr->payload_size = block->payload_size - sizeof(block_t) - payload_size;
        if(block_tail == block){
            block_tail = new_block_ptr;
        }
        // 更新block的数据
        block->payload_size = payload_size;
        // 重构free_list
        drop_from_free_list(block);
        add_to_free_list(block);
        add_to_free_list(new_block_ptr);
        // 重构list
        new_block_ptr->next = block->next;
        block->next = new_block_ptr;
        new_block_ptr->prev = block;

        // 测试
        // printf("[分裂完成]分成了%lu和%lu大小的两块\n",block->payload_size, new_block_ptr->payload_size);
        // print_list();
        // print_free_list();
    }
    return block;
}

void * ff_malloc(size_t size){
    // 首先尝试从空闲链表中的空闲块中分配
    block_t* block_ptr = find_fit_ff(size);

    if(block_ptr == NULL){  // 此时空闲链表中没有合适的块，所以需要进行拓展
        block_ptr = extend_heap(size);
    }else{  // 从空闲链表中找到合适的块
        // // 如果该空闲块足够大，那么尝试将其分裂
        // block_ptr = splitBlock(block_ptr, size);
        // 从空闲链表中删除这一空闲块
        drop_from_free_list(block_ptr);

        block_ptr->is_allocated = 1;
    }
    calFree();
    return (unsigned char*)block_ptr + sizeof(block_t);
}

void * bf_malloc(size_t size){
    // 首先尝试从空闲链表中的空闲块中分配
    block_t* block_ptr = find_fit_bf(size);

    if(block_ptr == NULL){  // 此时空闲链表中没有合适的块，所以需要进行拓展
        block_ptr = extend_heap(size);
    }else{  // 从空闲链表中找到合适的块
        // // 如果该空闲块足够大，那么尝试将其分裂
        // block_ptr = splitBlock(block_ptr, size);
        // 从空闲链表中删除这一空闲块
        drop_from_free_list(block_ptr);

        block_ptr->is_allocated = 1;
    }
    calFree();
    return (unsigned char*)block_ptr + sizeof(block_t);
}

void ff_free(void * ptr){
    block_free((block_t*)((unsigned char*)ptr - sizeof(block_t)));
    calFree();
}


void bf_free(void * ptr){
    INFO("bf_free()\n");
    block_free((block_t*)((unsigned char*)ptr - sizeof(block_t)));
    calFree();
}

unsigned long get_largest_free_data_segment_size() {
  return largest_free_size;
}

unsigned long get_total_free_size() {
  return free_size_sum;
}