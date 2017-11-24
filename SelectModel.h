#ifndef __SELECT_MODEL_H__
#define __SELECT_MODEL_H__

#include <pthread.h>
#include <sys/select.h>

#include "MultiPlexer.h"

class CSelectModel: public IMultiPlexer
{
    public:
        int start();
        int set_timeout(int milli_sec);
        static CSelectModel *instance();

    public:
        virtual int set_read_fd(int fd, IRwComponent *component);
        virtual int set_write_fd(int fd, IRwComponent *component);
        virtual int clear_read_fd(int fd);
        virtual int clear_write_fd(int fd);
        virtual int clear_read_fd_set();
        virtual int clear_write_fd_set();
        virtual ~CSelectModel();

    private:
        CSelectModel();

    private:
        int m_timeout;
        fd_set m_read_set;
        fd_set m_write_set;
        IRwComponent *m_components[FD_SETSIZE];

        static CSelectModel *m_inst;
        static pthread_mutex_t m_mutex;
};

#endif //__SELECT_MODEL_H__
