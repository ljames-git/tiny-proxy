#include "common.h"
#include "ThreadMessageQueue.h"

CThreadMessageQueue::CThreadMessageQueue():
    m_size(0),
    m_capacity(0),
    m_message_buf(NULL)
{
}

CThreadMessageQueue::CThreadMessageQueue(int size):
    m_size(0),
    m_capacity(0),
    m_message_buf(NULL)
{
    if (size > 0)
    {
        m_message_buf = new char* [size];
        m_capacity = size;
    }
}

CThreadMessageQueue::~CThreadMessageQueue()
{
    if (m_message_buf)
        delete []m_message_buf;
}

int CThreadMessageQueue::enqueue(char *msg)
{
    if (!msg || !m_message_buf)
        return -1;

    return 0;
}

char * CThreadMessageQueue::dequeue()
{
    return 0;
}

bool CThreadMessageQueue::is_empty()
{
    return m_size <= 0;
}

bool CThreadMessageQueue::is_full()
{
    return m_size >= m_capacity;
}
