#ifndef __PIPE_LINE_H__
#define __PIPE_LINE_H__

class CPipeLine
{
public:
    CPipeLine();
    CPipeLine(int thread_num);
    virtual ~CPipeLine();

public:
    int set_task_num(int num);
    int get_task_num();
    int set_next(CPipeLine *task_pool);
    CPipeLine * get_next();
    int start();
    int start(int num);

private:
    bool m_is_active;
    int m_thread_num;
    CPipeLine *m_next;
};

#endif //__PIPE_LINE_H__
