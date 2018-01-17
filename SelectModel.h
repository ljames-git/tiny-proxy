#ifndef __SELECT_MODEL_H__
#define __SELECT_MODEL_H__

#include <list>

#include <pthread.h>
#include <sys/select.h>

#include "RwObject.h"
#include "PipeLine.h"
#include "MultiPlexer.h"

class CSelectModel: public IMultiPlexer
{
public:
    ~CSelectModel();

    // singleton
    static CSelectModel *instance();

public:
    // implementations of IMultiPlexer interface
    virtual int run(void *msg, void ***plist, int *psize);
    virtual int set_pipe_line(CPipeLine *pipe_line);
    virtual int get_pipe_line_mode();
    virtual int clear_fd(int fd);
    virtual int set_timeout(int milli_sec);
    virtual int set_read_fd(int fd, IRwComponent *component);
    virtual int set_write_fd(int fd, IRwComponent *component);

private:
    CSelectModel();

private:
    // model timeout, millisecond
    int m_timeout;
    fd_set m_read_set;
    fd_set m_write_set;
    IRwComponent *m_components[FD_SETSIZE];
    CPipeLine *m_pipe_line;
    void *m_msg_list[MSG_LIST_MAX_SIZE];

    static CSelectModel *m_inst;
    static pthread_mutex_t m_mutex;
};

#endif //__SELECT_MODEL_H__
