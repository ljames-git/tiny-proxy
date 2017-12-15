#ifndef __COMMON_H__
#define __COMMON_H__

#include <time.h>
#include <stdio.h>

#define LOG_LEVEL_INFO "INFO"
#define LOG_LEVEL_DEBUG "DEBUG"
#define LOG_LEVEL_WARN "WARN"
#define LOG_LEVEL_ERROR "ERROR"

#define LOG_TAG(level) \
{\
    time_t t = time(NULL);\
    struct tm *local = localtime(&t); \
    fprintf(stderr, "%04d-%02d-%02d %02d:%02d:%02d [%s] ",\
            local->tm_year + 1900,\
            local->tm_mon + 1,\
            local->tm_mday + 1,\
            local->tm_hour,\
            local->tm_min,\
            local->tm_sec,\
            level);\
}

#define LOG_INFO(fmt, arg...) \
{\
    LOG_TAG(LOG_LEVEL_INFO);\
    fprintf(stderr, fmt, ##arg);\
    fprintf(stderr, "\n");\
}
#define LOG_DEBUG(fmt, arg...) \
{\
    LOG_TAG(LOG_LEVEL_DEBUG);\
    fprintf(stderr, fmt, ##arg);\
    fprintf(stderr, "\n");\
}
#define LOG_WARN(fmt, arg...) \
{\
    LOG_TAG(LOG_LEVEL_WARN);\
    fprintf(stderr, fmt, ##arg);\
    fprintf(stderr, "\n");\
}
#define LOG_ERROR(fmt, arg...) \
{\
    LOG_TAG(LOG_LEVEL_ERROR);\
    fprintf(stderr, fmt, ##arg);\
    fprintf(stderr, "\n");\
}


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
