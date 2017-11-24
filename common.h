#ifndef __COMMON_H__
#define __COMMON_H__

#include <time.h>

#define LOG_LEVEL_INFO "INFO"
#define LOG_LEVEL_DEBUG "DEBUG"
#define LOG_LEVEL_WARN "WARN"
#define LOG_LEVEL_ERROR "ERROR"

#define LOG_STREAM_ERR(level, str) \
{\
    time_t t = time(NULL);\
    struct tm *local = localtime(&t); \
    fprintf(stderr, "%04d-%02d-%02d %02d:%02d:%02d [%s] %s\n",\
            local->tm_year + 1900,\
            local->tm_mon + 1,\
            local->tm_mday + 1,\
            local->tm_hour,\
            local->tm_min,\
            local->tm_sec,\
            level,\
            str);\
}

#define LOG_INFO(str) LOG_STREAM_ERR(LOG_LEVEL_INFO, str)
#define LOG_DEBUG(str) LOG_STREAM_ERR(LOG_LEVEL_DEBUG, str)
#define LOG_WARN(str) LOG_STREAM_ERR(LOG_LEVEL_WARN, str)
#define LOG_ERROR(str) LOG_STREAM_ERR(LOG_LEVEL_ERROR, str)


#define ERROR_ON_NEG(func) \
{\
    int r = (func);\
    if (r < 0)\
    {\
        LOG_ERROR("error on: "#func);\
        return -9999;\
    }\
}

#define ASSIGN_AND_ERROR_ON_NEG(ret, func) \
{\
    int r = (func);\
    if (r < 0)\
    {\
        LOG_ERROR("error on: "#func);\
        return -9999;\
    }\
    ret = r;\
}

#endif //__COMMON_H__
