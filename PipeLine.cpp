#include "common.h"
#include "PipeLine.h"

struct thread_info_t
{
    int type;
    void *obj;
};

CPipeLine::CPipeLine():
    m_is_active(false),
    m_thread_num(1),
    m_next(NULL),
    m_threads(NULL),
    m_msg_queue(1024)
{
    m_threads = new pthread_t[m_thread_num];
}

CPipeLine::CPipeLine(int thread_num, int queue_size):
    m_is_active(false),
    m_thread_num(1),
    m_next(NULL),
    m_threads(NULL),
    m_msg_queue(queue_size)
{
    if (thread_num > 0)
    {
        m_thread_num = thread_num;
        m_threads = new pthread_t[m_thread_num];
    }
}

CPipeLine::~CPipeLine()
{
    if (m_threads)
    {
        delete []m_threads;
    }
}

int CPipeLine::set_task_num(int num)
{
    if (m_is_active || num <= 0)
        return -1;

    m_thread_num = num;
    return 0;
}

int CPipeLine::get_task_num()
{
    return m_thread_num;
}

int CPipeLine::set_next(CPipeLine *task_pool)
{
    if (!task_pool)
        return -1;

    m_next = task_pool;
    return 0;
}

CPipeLine * CPipeLine::get_next()
{
    return m_next;
}

int CPipeLine::start(int num)
{
    if (set_task_num(num) < 0)
        return -1;

    return start();
}

int CPipeLine::start()
{
    for (int i = 0; i < m_thread_num; i++)
    {
        thread_info_t *t = new thread_info_t;
        t->type = 0;
        t->obj = this;
        pthread_create(m_threads + i, NULL, routine, t);
    }
    return 0;
}

int CPipeLine::start(IRunnable *runnable)
{
    for (int i = 0; i < m_thread_num; i++)
    {
        thread_info_t *t = new thread_info_t;
        t->type = 1;
        t->obj = runnable;
        runnable->set_pipe_line(this);
        pthread_create(m_threads + i, NULL, routine, t);
    }
    return 0;
}

int CPipeLine::join()
{
    if (m_thread_num <= 0 || m_threads == NULL)
        return -1;

    for (int i = 0; i < m_thread_num; i++)
        pthread_join(m_threads[i], NULL);

    return 0;
}

int CPipeLine::process()
{
    return 0;
}

void * CPipeLine::routine(void * arg)
{
    if (arg == NULL)
        return NULL;

    thread_info_t *t = (thread_info_t *)arg;
    if (t->type == 0)
    {
        CPipeLine *inst = (CPipeLine *)t->obj;
        delete t;
        if (inst)
            inst->process();
    }
    else
    {
        IRunnable *runnable = (IRunnable *)t->obj;
        delete t;
        if (runnable)
            runnable->run();
    }
    return NULL;
}
