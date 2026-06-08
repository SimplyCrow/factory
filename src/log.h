#ifndef LOG_H
#define LOG_H

#ifndef LOG_TITLE
#define LOG_TITLE "LOG"
#endif

#include "unused.h"

#define LOG(fmt)        printf(LOG_TITLE"[%4zu]: "fmt"\n", _TOTAL_LOG_COUNT++)
#define LOGF(fmt, ...)  printf(LOG_TITLE"[%4zu]: "fmt"\n", _TOTAL_LOG_COUNT++, __VA_ARGS__)
UNUSED_VARIABLE static size_t _TOTAL_LOG_COUNT = 0;

#endif // LOG_H
