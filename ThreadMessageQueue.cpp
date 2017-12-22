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
    pthread_cond_init(&m_cond, NULL);
    pthread_mutex_init(&m_mutex, NULL);
}

CThreadMessageQueue::CThreadMessageQueue(int size):
    m_front(0),
    m_end(0),
    m_capacity(0),
    m_message_buf(NULL)
{
    if (size > 0)
    {
        pthread_cond_init(&m_cond, NULL);
        pthread_mutex_init(&m_mutex, NULL);
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

    int ret = -1;
    pthread_mutex_lock(&m_mutex);
    if (!is_full())
    {
        m_message_buf[m_end] = msg;
        m_end = (m_end + 1) % m_capacity;
        ret = 0;
    }
    pthread_cond_signal(&m_cond);
    pthread_mutex_unlock(&m_mutex);

    return ret;
}

char * CThreadMessageQueue::dequeue()
{
    if (m_capacity <= 0)
        return NULL;

    pthread_mutex_lock(&m_mutex);
    while (is_empty())
        pthread_cond_wait(&m_cond, &m_mutex);
    char * res = m_message_buf[m_front];
    m_front = (m_front + 1) % m_capacity;
    pthread_mutex_unlock(&m_mutex);

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
