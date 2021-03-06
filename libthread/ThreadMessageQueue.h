#ifndef __THREAD_MESSAGE_QUEUE_H__
#define __THREAD_MESSAGE_QUEUE_H__

#include <pthread.h>
#include "MessageQueue.h"

class CThreadMessageQueue : public IMessageQueue
{
public:
    CThreadMessageQueue();
    CThreadMessageQueue(int size);
    virtual ~CThreadMessageQueue();

public:
    virtual int enqueue(void *msg);
    virtual void * dequeue();
    virtual bool is_empty();
    virtual bool is_full();

private:
    int m_front;
    int m_end;
    int m_capacity;
    char ** m_message_buf;
    pthread_cond_t m_cond;
    pthread_mutex_t m_mutex;
};

#endif //__THREAD_MESSAGE_QUEUE_H__
