#include "common.h"
#include "PipeLine.h"

CPipeLine::CPipeLine():
    m_runnable(NULL),
    m_is_active(false),
    m_thread_num(1),
    m_next(NULL),
    m_threads(NULL),
    m_msg_queue(1024)
{
    m_threads = new pthread_t[m_thread_num];
}

CPipeLine::CPipeLine(int thread_num, int queue_size):
    m_runnable(NULL),
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

int CPipeLine::start(IRunnable *runnable)
{
    for (int i = 0; i < m_thread_num; i++)
    {
        m_runnable = runnable;
        runnable->set_pipe_line(this);
        pthread_create(m_threads + i, NULL, routine, this);
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

void * CPipeLine::routine(void * arg)
{
    if (arg == NULL)
        return NULL;

    CPipeLine *pipe_line = (CPipeLine *)arg;
    if (pipe_line == NULL)
        return NULL;

    IRunnable *runnable = pipe_line->m_runnable;
    if (runnable == NULL)
        return NULL;

    int mode = runnable->get_pipe_line_mode();
    for (;;)
    {
        void *msg = NULL;
        void **plist = NULL;
        int list_size = 0;
        if (mode != PIPE_LINE_MODE_HEAD)
        {
            msg = pipe_line->m_msg_queue.dequeue();
            if (!msg)
                continue;
        }
        if (runnable->run(msg, &plist, &list_size) < 0)
        {
            LOG_WARN("run error");
            continue;
        }
        if (mode != PIPE_LINE_MODE_TAIL)
        {
            if (pipe_line->m_next == NULL)
            {
                LOG_ERROR("next pipe line not set on mode: %d", mode);
                return NULL;
            }
            for (int i = 0; i < list_size; i++)
            {
                pipe_line->m_next->m_msg_queue.enqueue(plist[i]);
            }
        }
    }
    return NULL;
}
