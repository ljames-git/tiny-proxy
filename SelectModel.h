#ifndef __SELECT_MODEL_H__
#define __SELECT_MODEL_H__

#include <deque>

#include <pthread.h>
#include <sys/select.h>

#include "RwObject.h"
#include "MultiPlexer.h"

class CSelectModel: public IMultiPlexer
{
public:
    virtual ~CSelectModel();

    // singleton
    static CSelectModel *instance();

public:
    // implementations of IMultiPlexer interface
    virtual int start();
    virtual int clear_fd(int fd);
    virtual int set_timeout(int milli_sec);
    virtual int set_read_fd(int fd, IRwComponent *component);
    virtual int clear_read_fd(int fd);
    virtual int write(int fd, const char *buf, int size, IRwComponent *component);
    virtual int chunk_write(int fd, const char *buf, int size, IRwComponent *component, bool is_last = true);

private:
    CSelectModel();
    //int do_write(int fd);
    int do_chunk_write(int fd);

private:
    // model timeout, millisecond
    int m_timeout;
    fd_set m_read_set;
    fd_set m_write_set;
    IRwComponent *m_components[FD_SETSIZE];
    std::deque<CRwObject *> m_rw_obj_deque[FD_SETSIZE];

    static CSelectModel *m_inst;
    static pthread_mutex_t m_mutex;
};

#endif //__SELECT_MODEL_H__
