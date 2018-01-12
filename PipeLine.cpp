#include "common.h"
#include "PipeLine.h"

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
        pthread_create(m_threads + i, NULL, routine, this);
    return 0;
}

int CPipeLine::start(IRunnable *runnable)
{
    for (int i = 0; i < m_thread_num; i++)
        pthread_create(m_threads + i, NULL, routine, this);
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
    CPipeLine *inst = NULL;
    IRunnable *runnable = dynamic_cast<IRunnable *>(arg);
    if (!runnable)
        inst = (CPipeLine *)arg;

    if (inst != NULL)
        inst->process();
    else if (runnable)
        runnable->run();

    return NULL;
}
