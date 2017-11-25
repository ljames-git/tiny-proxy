#ifndef __MULTI_PLEXER_H__
#define __MULTI_PLEXER_H__

#include "RwComponent.h"

class IMultiPlexer
{
    public:
        virtual int set_read_fd(int fd, IRwComponent *component) = 0;
        virtual int set_write_fd(int fd, IRwComponent *component) = 0;
        virtual int clear_read_fd(int fd) = 0;
        virtual int clear_write_fd(int fd) = 0;
        virtual int set_timeout(int milli_sec) = 0;
        virtual int clear_read_fd_set() = 0;
        virtual int clear_write_fd_set() = 0;
        virtual int start() = 0;
};

#endif //__MULTI_PLEXER_H__
