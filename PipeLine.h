#ifndef __PIPE_LINE_H__
#define __PIPE_LINE_H__

#include "ThreadMessageQueue.h"

class CPipeLine
{
public:
    CPipeLine();
    CPipeLine(int thread_num, int queue_size = 1024);
    virtual ~CPipeLine();

    int set_task_num(int num);
    int get_task_num();
    int set_next(CPipeLine *task_pool);
    CPipeLine * get_next();
    int start();
    int start(int num);
    int join();

public:
    virtual int process();

private:
    static void * routine (void *arg);

    bool m_is_active;
    int m_thread_num;
    CPipeLine *m_next;
    pthread_t *m_threads;
    CThreadMessageQueue m_msg_queue;
};

#endif //__PIPE_LINE_H__
