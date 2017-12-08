#ifndef __MULTI_PLEXER_H__
#define __MULTI_PLEXER_H__

#include "RwComponent.h"

class IMultiPlexer
{
public:
    virtual int start() = 0;
    virtual int clear_fd(int fd) = 0;
    virtual int set_timeout(int milli_sec) = 0;
    virtual int set_read_fd(int fd, IRwComponent *component) = 0;
    virtual int write(int fd, const char *buf, int size, IRwComponent *component) = 0;
};

#endif //__MULTI_PLEXER_H__
