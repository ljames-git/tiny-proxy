#ifndef __HTTP_CLIENT_H__
#define __HTTP_CLIENT_H__

#include "Runnable.h"

class CHttpClient: public IRunnable
{
public:
    CHttpClient();
    ~CHttpClient();

    virtual int run();
    virtual int set_pipe_line(CPipeLine *pipe_line);

private:
    CPipeLine *m_pipe_line;
};

#endif //__HTTP_CLIENT_H__
