#ifndef __HTTP_CLIENT_H__
#define __HTTP_CLIENT_H__

#include "Runnable.h"

class CHttpClient: public IRunnable
{
public:
    CHttpClient();
    ~CHttpClient();

    virtual int run(void *msg, void ***plist, int *psize);
    virtual int set_pipe_line(CPipeLine *pipe_line);
    virtual int get_pipe_line_mode();

private:
    CPipeLine *m_pipe_line;
};

#endif //__HTTP_CLIENT_H__
