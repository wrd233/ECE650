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

    int* p1 = (int*)bf_malloc(10);
    int* p2 = (int*)bf_malloc(20);
    int* p3 = (int*)bf_malloc(3000);
    int* p4 = (int*)bf_malloc(40);
    int* p5 = (int*)bf_malloc(50);
  
    bf_free(p1);
    bf_free(p3);
    p1 = (int*)bf_malloc(1200);
    p3 = (int*)bf_malloc(10);
}
