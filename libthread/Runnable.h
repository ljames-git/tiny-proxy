#ifndef __RUNNABLE_H__
#define __RUNNABLE_H__

#define MSG_LIST_MAX_SIZE 1024

class IRunnable
{
public:
    virtual int run(void *msg, void ***plist, int *psize) = 0;
};

#endif //__RUNNABLE_H__
