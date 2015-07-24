#ifndef BV_COMMON_H
#define BV_COMMON_H

#include <stdio.h>

#define max(x, y) x < y ? y : x 
#define min(x, y) x < y ? x : y 
#define likely(x)    __builtin_expect (!!(x), 1)
#define unlikely(x)  __builtin_expect (!!(x), 0)

#ifndef ERR
#define ERR
#endif

#ifndef WARNING
#define WARNING
#endif

#ifndef INFO
#define INFO
#endif

#ifndef DEBUG
#define DEBUG
#endif

#define LOG(x, fmt, ...) \
  printf(#x " %s (%u):" fmt, __func__, __LINE__, ## __VA_ARGS__)
#endif
