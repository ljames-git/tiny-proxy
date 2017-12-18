#ifndef __PIPE_LINE_H__
#define __PIPE_LINE_H__

#include "TaskPool.h"

class CPipeLine : public ITaskPool
{
public:
    CPipeLine();
    CPipeLine(int thread_num);
    virtual ~CPipeLine();

public:
    virtual int set_task_num(int num);
    virtual int get_task_num();
    virtual int set_next(ITaskPool *task_pool);
    virtual ITaskPool * get_next();
    virtual int start();
    virtual int start(int num);

private:
    bool m_is_active;
    int m_thread_num;
    ITaskPool *m_next;
};

#endif //__PIPE_LINE_H__
