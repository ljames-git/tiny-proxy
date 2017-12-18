#include "common.h"
#include "TcpServer.h"
#include "HttpServer.h"

#ifndef MULTI_THREAD_VERSION
int main(int argc, char ** argv)
{
    int port = 8888;
    CTcpServer *s = new CHttpServer(port);
    if (s == NULL)
    {
        LOG_ERROR("failed to create httpserver, bind port: %d", port);
        return -1;
    }

    if (s->start() == 0 && s->get_model()->start() == 0)
    {
        LOG_INFO("HTTP SERVER START SUCCESSFULLY");
    }
    else
    {
        LOG_ERROR("HTTP SERVER START ERROR");
        return -2;
    }

    return 0;
}
#else
#endif //MULTI_THREAD_VERSION
