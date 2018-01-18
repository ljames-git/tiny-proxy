#ifndef __MESSAGE_QUEUE_H__
#define __MESSAGE_QUEUE_H__

class IMessageQueue
{
public:
    virtual int enqueue(void *msg) = 0;
    virtual void * dequeue() = 0;
    virtual bool is_empty() = 0;
    virtual bool is_full() = 0;
};

#endif //__MESSAGE_QUEUE_H__
