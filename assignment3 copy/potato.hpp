#include <stdio.h>
#include <string.h>

#include <cstdio>
#include <cstdlib>
#include <vector>
class Potato {
    public:
    int num_hops;
    int count;
    int path[1024];
    Potato():num_hops(0), count(0){
        memset(path, 0, sizeof(path));
    }
    Potato(int num_hops) : num_hops(num_hops), count(0){
        memset(path, 0, sizeof(path));
    }
};