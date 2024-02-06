#include "my_malloc.h"
#include <stdio.h>

int main(){
    // int* p1 = (int*)ff_malloc(10);
    // int* p2 = (int*)ff_malloc(20);
    // int* p3 = (int*)ff_malloc(30);
    // int* p4 = (int*)ff_malloc(40);
    // int* p5 = (int*)ff_malloc(50);
  
    // ff_free(p1);
    // p1 = (int*)ff_malloc(10);

    /*===对于free_list的测试====*/
    block_t* b1 = extend_heap(10);
    block_t* b2 = extend_heap(20);
    block_t* b3 = extend_heap(30);
    block_t* b4 = extend_heap(40);
    block_t* b5 = extend_heap(50);
    block_t* b6 = extend_heap(60);

    add_to_free_list(b1);
    add_to_free_list(b2);
    add_to_free_list(b3);
    add_to_free_list(b4);
    add_to_free_list(b5);
    add_to_free_list(b6);

    drop_from_free_list(b1);
    drop_from_free_list(b2);
    drop_from_free_list(b3);
    drop_from_free_list(b4);
    drop_from_free_list(b5);
    drop_from_free_list(b6);

    block_t* b7 = extend_heap(10);
    add_to_free_list(b7);
    drop_from_free_list(b7);

    // int* p1 = (int*)bf_malloc(10);
    // int* p2 = (int*)bf_malloc(20);
    // int* p3 = (int*)bf_malloc(3000);
    // int* p4 = (int*)bf_malloc(40);
    // int* p5 = (int*)bf_malloc(50);
  
    // bf_free(p1);
    // bf_free(p3);
    // p1 = (int*)bf_malloc(1200);
    // p3 = (int*)bf_malloc(10);
}
