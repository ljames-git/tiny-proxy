#ifndef __TASK_POOL_H__
#define __TASK_POOL_H__

class ITaskPool
{
public:
    virtual int set_task_num(int num) = 0;
    virtual int get_task_num() = 0;
    virtual int set_next(ITaskPool *task_pool) = 0;
    virtual ITaskPool * get_next() = 0;
    virtual int start() = 0;
    virtual int start(int num) = 0;
};

#endif //__TASK_POOL_H__
