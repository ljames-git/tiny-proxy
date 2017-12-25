#ifndef __MSG_H__
#define __MSG_H__

#include "HttpServer.h"

struct msg_t
{
    CHttpServer *server;
    http_task_t *task;
};

#endif //__MSG_H__
