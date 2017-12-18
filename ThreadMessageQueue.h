#ifndef __THREAD_MESSAGE_QUEUE_H__
#define __THREAD_MESSAGE_QUEUE_H__

#include "MessageQueue.h"

class CThreadMessageQueue : public IMessageQueue
{
public:
    CThreadMessageQueue();
    CThreadMessageQueue(int size);
    virtual ~CThreadMessageQueue();

public:
    virtual int enqueue(char *msg);
    virtual char * dequeue();
    virtual bool is_empty();
    virtual bool is_full();

private:
    int m_size;
    int m_capacity;
    char ** m_message_buf;
};

#endif //__THREAD_MESSAGE_QUEUE_H__
