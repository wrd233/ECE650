#include "my_malloc.h"
#include <stdint.h>
#include <unistd.h>
#include <assert.h>

unsigned long largest_free_size = 0;    // TODO: 添加这个的全局逻辑
unsigned long free_size_sum = 0;        // TODO: 添加这个的全局逻辑

static void calFree(){
    if(free_head == NULL){
        return;
    }
    block_t* ptr = free_head;
    free_size_sum = 0;
    largest_free_size = 0;
    while(ptr->free_next != free_head){
        free_size_sum += ptr->payload_size;
        largest_free_size = largest_free_size > ptr->payload_size ? largest_free_size : ptr->payload_size;
        ptr = ptr->free_next;
    }
    free_size_sum += ptr->payload_size;
    largest_free_size = largest_free_size > ptr->payload_size ? largest_free_size : ptr->payload_size;
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
    // if(block_head == NULL){
    //     printf("链表中尚未有元素");
    //     return;
    // }
    // printf("=================\n");
    // for(block_t* ptr = block_head; ptr != NULL; ptr = get_next_block(ptr)){
    //     printf("payload_size = %lu, is_allocated = %d\n",ptr->payload_size, ptr->is_allocated);
    // }
    // printf("=================\n");
}

void print_free_list(){
    if(free_head == NULL){
        INFO("free list中尚未有元素\n");
        return;
    }
    INFO("~~~~~~~~~~~~~~~~~~\n");
    block_t* ptr = free_head;
    while(ptr != NULL){
        INFO("%lu/%d->",ptr->payload_size, ptr->is_allocated);
        ptr = ptr->free_next;
    }
    INFO("\n~~~~~~~~~~~~~~~~~~\n");
}

// 使用first fit的策略找到合适的block
static block_t* find_fit_ff(size_t payload_size){
    // printf("尝试寻找ff，当前寻找的size为%lu\n",payload_size);
    
    block_t* current = free_head;

    while (current != NULL) {
        // 判断该节点能否承载payload
        if(current->payload_size >= payload_size){
            return current;
        }
        current = current->free_next;

        // 如果当前节点等于头结点，说明已经遍历完整个链表
        if (current == free_head) {
            break;
        }
    }

    return NULL;
}

// 使用best fit的策略找到合适的block
static block_t* find_fit_bf(size_t payload_size){
    // printf("尝试寻找bf，当前寻找的size为%lu\n",payload_size);

    block_t* current = free_head;
    block_t* best_fit_ptr = NULL;

    while (current != NULL) {
        // 判断该节点能否承载payload
        if(current->payload_size >= payload_size){
            if(best_fit_ptr == NULL){
                best_fit_ptr = current;
            }else{
                best_fit_ptr = best_fit_ptr->payload_size < current->payload_size ? best_fit_ptr : current;
            }
        }

        current = current->free_next;

        // 如果当前节点等于头结点，说明已经遍历完整个链表
        if (current == free_head) {
            break;
        }
    }

    return best_fit_ptr;
}

/*
 *  注：从空闲链表中删除不意味着is_allocated需要置为1
*/
void drop_from_free_list(block_t* block){
    assert(block!=NULL);
    assert(block->is_allocated == 0);

    // 将当前节点从双向链表中去除
    if(free_head == block && block->free_prev == NULL){  // 此时free list中只有一个元素
        assert(block->free_next == NULL);
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
    assert(block != NULL);
    assert(block->free_prev==NULL && block->free_next == NULL);

    if(block->is_allocated == 1){
        block->is_allocated = 0;
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

// static void drop_from_list(block_t* block){
//     assert(block!=NULL);
//     assert(block->next!=NULL && block->prev!=NULL);

//     // 将当前节点从双向链表中去除
//     if(block->next == block){  // 此时free list中只有一个元素
//         block_head = NULL;
//         block->next = NULL;
//         block->prev = NULL;
//     }else{
//         if (block_head == block) {
//             block_head = block->next;
//         }
//         block->prev->next = block->next;
//         block->next->prev = block->prev;
//     }

//     // print_free_list();
// }

// static void add_to_list(block_t* block){
//     assert(block != NULL);
//     assert(block->prev==NULL && block->next == NULL);

//     if(block_head != NULL){
//         block->next = block_head;
//         block->prev = block_head->prev;
//         block_head->prev = block;
//         block->prev->next = block;
//     }else{
//         block->next = block;
//         block->prev = block;
//     }

//     block_head = block;
//     // print_free_list();
// }

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
    // printf("调用了free\n");
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
        // 如果该空闲块足够大，那么尝试将其分裂
        block_ptr = splitBlock(block_ptr, size);
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
        // 如果该空闲块足够大，那么尝试将其分裂
        block_ptr = splitBlock(block_ptr, size);
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
    block_free((block_t*)((unsigned char*)ptr - sizeof(block_t)));
    calFree();
}

unsigned long get_largest_free_data_segment_size() {
  return largest_free_size;
}

unsigned long get_total_free_size() {
  return free_size_sum;
}