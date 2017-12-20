#include "common.h"
#include "ThreadMessageQueue.h"

CThreadMessageQueue::CThreadMessageQueue():
    m_front(0),
    m_end(0),
    m_capacity(0),
    m_message_buf(NULL)
{
    m_capacity = 1024;
    m_message_buf = new char *[1024];
}

CThreadMessageQueue::CThreadMessageQueue(int size):
    m_front(0),
    m_end(0),
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
    if (!msg || !m_message_buf || m_capacity <= 0)
        return -1;

    m_message_buf[m_end] = msg;
    m_end = (m_end + 1) % m_capacity;
    return 0;
}

char * CThreadMessageQueue::dequeue()
{
    if (is_empty() || m_capacity <= 0)
        return NULL;

    char * res = m_message_buf[m_front];
    m_front = (m_front + 1) % m_capacity;
    return res;
}

bool CThreadMessageQueue::is_empty()
{
    return m_end == m_front;
}

bool CThreadMessageQueue::is_full()
{
    if (m_capacity <= 0)
        return true;

    int end = (m_end + 1) % m_capacity;
    return end == m_front;
}
