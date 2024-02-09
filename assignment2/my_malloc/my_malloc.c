#include "my_malloc.h"
#include <stdint.h>
#include <unistd.h>
#include <assert.h>
#include <pthread.h>

block_t* block_head_lock = NULL;
block_t* block_tail_lock = NULL;
block_t* free_head_lock = NULL;

__thread block_t* block_head_nolock = NULL;
__thread block_t* block_tail_nolock = NULL;
__thread block_t* free_head_nolock = NULL;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

unsigned long largest_free_size = 0;    // TODO: 添加这个的全局逻辑
unsigned long free_size_sum = 0;        // TODO: 添加这个的全局逻辑

static block_t* get_next_block(block_t* curr){
    return curr->next;
}

void print_list(global_t* global_list_info){
    block_t* block_head = *global_list_info->block_head_ptr;
    block_t* block_tail = *global_list_info->block_tail_ptr;
    block_t* free_head = *global_list_info->free_head_ptr;

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

void print_free_list(global_t* global_list_info){
    block_t* block_head = *global_list_info->block_head_ptr;
    block_t* block_tail = *global_list_info->block_tail_ptr;
    block_t* free_head = *global_list_info->free_head_ptr;

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

// 使用best fit的策略找到合适的block
static block_t* find_fit_bf(size_t payload_size, global_t* global_list_info){
    block_t* block_head = *global_list_info->block_head_ptr;
    block_t* block_tail = *global_list_info->block_tail_ptr;
    block_t* free_head = *global_list_info->free_head_ptr;

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
// TODO: 改写
void drop_from_free_list(block_t* block, global_t* global_list_info){
    block_t* block_head = *global_list_info->block_head_ptr;
    block_t* block_tail = *global_list_info->block_tail_ptr;
    block_t* free_head = *global_list_info->free_head_ptr;

    INFO("drop_from_free_list()\n");
    assert(block!=NULL);
    assert(block->is_allocated == 0);
    assert(free_head != NULL);

    // 将当前节点从双向链表中去除
    if(free_head == block && block->free_next == NULL){  // 此时free list中只有一个元素
        assert(block->free_prev == NULL);
        *global_list_info->free_head_ptr = NULL;
    }else{
        fflush(stdout);
        assert(block->free_next!=NULL || block->free_prev!=NULL);
        if (free_head == block) {
            *global_list_info->free_head_ptr = block->free_next;
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
    print_free_list(global_list_info);
}

// TODO: 改写
void add_to_free_list(block_t* block, global_t* global_list_info){
    block_t* block_head = *global_list_info->block_head_ptr;
    block_t* block_tail = *global_list_info->block_tail_ptr;
    block_t* free_head = *global_list_info->free_head_ptr;

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
        (*global_list_info->free_head_ptr)->free_prev = block;
    }else{
        block->free_next = NULL;
        block->free_prev = NULL;
    }

    *global_list_info->free_head_ptr = block;
    print_free_list(global_list_info);
}

/*
 * 
*/
// TODO: 改写
void drop_from_list(block_t* block, global_t* global_list_info){
    block_t* block_head = *global_list_info->block_head_ptr;
    block_t* block_tail = *global_list_info->block_tail_ptr;
    block_t* free_head = *global_list_info->free_head_ptr;

    assert(block!=NULL);
    assert(block_head != block);    // 理论上是不会删除第一个块的，所以该函数内操作的block都有前序block
    assert(block->prev != NULL);
    assert(block_head != block || block_tail != block); //不可能将唯一的块删除

    if(block_tail == block){
        *global_list_info->block_tail_ptr = block->prev;
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
// TODO: 改写
void add_to_list_tail(block_t* block, global_t* global_list_info){
    block_t* block_head = *global_list_info->block_head_ptr;
    block_t* block_tail = *global_list_info->block_tail_ptr;
    block_t* free_head = *global_list_info->free_head_ptr;

    if(block_tail != NULL){
        (*global_list_info->block_tail_ptr)->next = block;
    }
    *global_list_info->block_tail_ptr = block;

    if(block_head == NULL){
        *global_list_info->block_head_ptr = block;
    }

    print_list(global_list_info);
}

// 如果原先的内存池不够了，那么就拓展制定大小的block
block_t* extend_heap(size_t payload_size, global_t* global_list_info, int is_sbrk_locked){
    block_t* block_head = *global_list_info->block_head_ptr;
    block_t* block_tail = *global_list_info->block_tail_ptr;
    block_t* free_head = *global_list_info->free_head_ptr;

    // printf("调用了extend\n");
    size_t size_sum = sizeof(struct block) + payload_size;
    block_t* new_ptr = NULL;
    if(is_sbrk_locked == 0){
        new_ptr = sbrk(size_sum);
    }else{
        pthread_mutex_lock(&lock);
        new_ptr = sbrk(size_sum);
        pthread_mutex_unlock(&lock);
    }

    new_ptr->payload_size = payload_size;
    new_ptr->is_allocated = 1;
    new_ptr->prev = block_tail;
    new_ptr->next = NULL;
    new_ptr->free_next = NULL;
    new_ptr->free_prev = NULL;

    // TODO:
    add_to_list_tail(new_ptr, global_list_info);

    return new_ptr;
}

// 收回内存的时候直接合并相邻的; 把新的空闲块加入到空闲链表当中(头插吧要不)
void block_free(block_t* block, global_t* global_list_info){
    block_t* block_head = *global_list_info->block_head_ptr;
    block_t* block_tail = *global_list_info->block_tail_ptr;
    block_t* free_head = *global_list_info->free_head_ptr;

    assert(block != NULL);
    assert(block->is_allocated == 1);
    assert(block->free_prev == NULL && block->free_next == NULL);

    // 首先将当前block的标志位清除
    block->is_allocated = 0;

    if(free_head == NULL){
        *global_list_info->free_head_ptr = block;
    }else{
        block_t* merged_block_ptr = block;
        block_t* prev_block = block->prev;
        block_t* next_block = block->next;
        // 尝试与prev的块合并

        if(prev_block != NULL && prev_block->is_allocated == 0){
            INFO("prev为free block, 尝试合并\n");
            drop_from_free_list(prev_block, global_list_info);
            drop_from_list(block, global_list_info);
            prev_block->payload_size += sizeof(block_t) + block->payload_size;
            merged_block_ptr = prev_block;
        }

        // 尝试与next的块合并
        if(next_block != NULL && next_block->is_allocated == 0){
            INFO("next为free block, 尝试合并\n");
            drop_from_free_list(next_block, global_list_info);
            drop_from_list(next_block, global_list_info);
            merged_block_ptr->payload_size += sizeof(block_t) + next_block->payload_size;
        }

        add_to_free_list(merged_block_ptr, global_list_info);
    }

    print_free_list(global_list_info);
    print_list(global_list_info);
}

// TODO: 将blcok分割成payload_size和剩下的部分，分割失败返回NULL
static block_t* splitBlock(block_t* block, size_t payload_size, global_t* global_list_info){
    block_t* block_head = *global_list_info->block_head_ptr;
    block_t* block_tail = *global_list_info->block_tail_ptr;
    block_t* free_head = *global_list_info->free_head_ptr;

    // TODO: 分割空闲块，并将分割之后的后半个block加入到空闲链表当中
    assert(block != NULL);
    assert(payload_size >= 0);
    if(free_head != block){
        assert(block->free_next != NULL || block->free_prev != NULL);
    }

    INFO("当前块的大小为%lu, 期望分配%lu\n",block->payload_size, payload_size);

    // TODO: 当下面两个内容相同的时候，会出现问题
    if(block->payload_size >= sizeof(block_t) + payload_size){
        INFO("尝试进行分裂\n");
        // 创建新的节点
        size_t offset = payload_size + sizeof(block_t);
        block_t* new_block_ptr = (block_t*)((unsigned char*)block + offset);
        new_block_ptr->free_prev = NULL;
        new_block_ptr->free_next = NULL;
        new_block_ptr->is_allocated = 0;
        new_block_ptr->payload_size = block->payload_size - sizeof(block_t) - payload_size;
        if(block_tail == block){
            *global_list_info->block_tail_ptr = new_block_ptr;
        }
        // 更新block的数据
        block->payload_size = payload_size;
        // 重构free_list
        drop_from_free_list(block, global_list_info);
        add_to_free_list(block, global_list_info);
        add_to_free_list(new_block_ptr, global_list_info);
        // 重构list
        new_block_ptr->next = block->next;
        block->next = new_block_ptr;
        new_block_ptr->prev = block;

        // 测试
        INFO("[分裂完成]分成了%lu和%lu大小的两块\n",block->payload_size, new_block_ptr->payload_size);
    }
    return block;
}

void * bf_malloc(size_t size, global_t* global_list_info, int is_sbrk_locked){
    block_t* block_head = *global_list_info->block_head_ptr;
    block_t* block_tail = *global_list_info->block_tail_ptr;
    block_t* free_head = *global_list_info->free_head_ptr;

    // 首先尝试从空闲链表中的空闲块中分配
    block_t* block_ptr = find_fit_bf(size, global_list_info);

    if(block_ptr == NULL){  // 此时空闲链表中没有合适的块，所以需要进行拓展
        block_ptr = extend_heap(size, global_list_info, is_sbrk_locked);
    }else{  // 从空闲链表中找到合适的块
        // // 如果该空闲块足够大，那么尝试将其分裂
        // block_ptr = splitBlock(block_ptr, size);
        // 从空闲链表中删除这一空闲块
        drop_from_free_list(block_ptr, global_list_info);

        block_ptr->is_allocated = 1;
    }
    return (unsigned char*)block_ptr + sizeof(block_t);
}


void bf_free(void * ptr, global_t* global_list_info){
    INFO("bf_free()\n");
    block_free((block_t*)((unsigned char*)ptr - sizeof(block_t)), global_list_info);
}

unsigned long get_largest_free_data_segment_size() {
  return largest_free_size;
}

unsigned long get_total_free_size() {
  return free_size_sum;
}

//Thread Safe malloc/free: locking version
void *ts_malloc_lock(size_t size){
    pthread_mutex_lock(&lock);
    global_t global_list_info = {
        .block_head_ptr = &block_head_lock,
        .block_tail_ptr = &block_tail_lock,
        .free_head_ptr = &free_head_lock
    };
    void* ptr = bf_malloc(size, &global_list_info, 0);
    pthread_mutex_unlock(&lock);

    return ptr;
}

void ts_free_lock(void *ptr){
    pthread_mutex_lock(&lock);
    global_t global_list_info = {
        .block_head_ptr = &block_head_lock,
        .block_tail_ptr = &block_tail_lock,
        .free_head_ptr = &free_head_lock
    };
    bf_free(ptr, &global_list_info);
    pthread_mutex_unlock(&lock);
}

//Thread Safe malloc/free: non-locking version
void *ts_malloc_nolock(size_t size){
    global_t global_list_info = {
        .block_head_ptr = &block_head_lock,
        .block_tail_ptr = &block_tail_lock,
        .free_head_ptr = &free_head_lock
    };
    void* ptr = bf_malloc(size, &global_list_info, 1);
    return ptr;
}

void ts_free_nolock(void *ptr){
    global_t global_list_info = {
        .block_head_ptr = &block_head_lock,
        .block_tail_ptr = &block_tail_lock,
        .free_head_ptr = &free_head_lock
    };
    bf_free(ptr, &global_list_info);
}