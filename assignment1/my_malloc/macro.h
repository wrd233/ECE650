#include <stdio.h>

#ifdef DEBUG
#define LOG(message) printf("[DEBUG] %s\n", message)
#else
#define LOG(message)
#endif