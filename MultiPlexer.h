#ifndef __MULTI_PLEXER_H__
#define __MULTI_PLEXER_H__

#include "RwComponent.h"
#include "Runnable.h"

class IMultiPlexer: public IRunnable
{
public:
    virtual int clear_fd(int fd) = 0;
    virtual int set_timeout(int milli_sec) = 0;
    virtual int set_read_fd(int fd, IRwComponent *component) = 0;
    virtual int set_write_fd(int fd, IRwComponent *component) = 0;
};

#endif //__MULTI_PLEXER_H__
