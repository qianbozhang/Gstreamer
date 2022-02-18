#ifndef _LOG_H_
#define _LOG_H_

#include <stdio.h>
#include <time.h>

#define DEBUG_ALWAYS  0
#define DEBUG_ERROR   1
#define DEBUG_WAR     2
#define DEBUG_INFO    3
#define DEBUG_DETAIL  4

#define CLASSNAME     "unknown"

#define SET_LOG_LEVEL(level)  do{ g_level = DEBUG_##level; }while(0);

static int g_level = DEBUG_ERROR;

//[s.ns] [level] CLASSNAME: filename:line:fun(): txt
typedef struct _PrintInfo
{
    const char *levelName;
    const char *className;
    const char *fileName;
    const char *function;
    int   line;
}PrintInfo_t;

#define INIT_PRINTINFO(level) {#level, CLASSNAME, __FILE__, __FUNCTION__, __LINE__}

#define LOG_PRINTF(level, fmt, ...)                             \
{                                                               \
    if(DEBUG_##level <= g_level){                               \
        PrintInfo_t info = INIT_PRINTINFO(level);               \
        char log_header[512];                                   \
        struct timespec time_={0 , 0};                          \
        clock_gettime(CLOCK_MONOTONIC, &time_);                 \
        sprintf(log_header, "[%8ld.%-10ld] [%6s] %s: %s:%d:%s(): ", \
            time_.tv_sec,                                       \
            time_.tv_nsec,                                      \
            info.levelName,                                     \
            info.className,                                     \
            info.fileName,                                      \
            info.line,                                          \
            info.function);                                     \
        printf("%s ", log_header);                              \
        printf(fmt "\n", ##__VA_ARGS__);                        \
    }                                                           \
}



#endif