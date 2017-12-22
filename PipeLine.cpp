#include "common.h"
#include "PipeLine.h"

CPipeLine::CPipeLine():
    m_is_active(false),
    m_thread_num(1),
    m_next(NULL),
    m_msg_queue(1024)
{
}

CPipeLine::CPipeLine(int thread_num):
    m_is_active(false),
    m_thread_num(1),
    m_next(NULL),
    m_msg_queue(1024)
{
    if (thread_num > 0)
        m_thread_num = thread_num;
}

CPipeLine::~CPipeLine()
{
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
    return 0;
}

int CPipeLine::message_proc(void *msg)
{
    return 0;
}
