#include <stdio.h>

#ifdef DEBUG
// #define INFO(...) \
// printf("[INFO] %s  %s:%d\n\t", __PRETTY_FUNCTION__, __FILE__, __LINE__); \
// printf(__VA_ARGS__); \
// printf("\n");
#define INFO(...) \
printf(__VA_ARGS__);
#else
#define INFO(...)
#endif