#ifndef __RUNNABLE_H__
#define __RUNNABLE_H__

#include "PipeLine.h"

class CPipeLine;

class IRunnable
{
public:
    virtual int run() = 0;
    virtual int set_pipe_line(CPipeLine *pipe_line) = 0;
};

#endif //__RUNNABLE_H__
