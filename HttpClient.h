#ifndef __HTTP_CLIENT_H__
#define __HTTP_CLIENT_H__

#include "PipeLine.h"

class CHttpClient: public CPipeLine
{
public:
    CHttpClient();
    CHttpClient(int thread_num, int queue_size = 1024);
    ~CHttpClient();

    virtual int process();
};

#endif //__HTTP_CLIENT_H__
