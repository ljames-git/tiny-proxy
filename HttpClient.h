#ifndef __HTTP_CLIENT_H__
#define __HTTP_CLIENT_H__

#include "Runnable.h"

class CHttpClient: public IRunnable
{
public:
    CHttpClient();
    ~CHttpClient();

    virtual int run(void *msg, void ***plist, int *psize);
};

#endif //__HTTP_CLIENT_H__
