#ifndef __RUNNABLE_H__
#define __RUNNABLE_H__

#define MSG_LIST_MAX_SIZE 1024

class CPipeLine;

class IRunnable
{
public:
    virtual int run(void *msg, void ***plist, int *psize) = 0;
    virtual int set_pipe_line(CPipeLine *pipe_line) = 0;
    virtual int get_pipe_line_mode() = 0;
};

#endif //__RUNNABLE_H__
