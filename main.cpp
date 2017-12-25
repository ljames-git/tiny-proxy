#include <signal.h>
#include "common.h"
#include "TcpServer.h"
#include "HttpServer.h"
#include "HttpClient.h"

#ifndef MULTI_THREAD_VERSION
int main(int argc, char ** argv)
{
    signal(SIGPIPE, SIG_IGN);

    CHttpClient *client = new CHttpClient(8);
    if (client == NULL)
    {
        LOG_ERROR("failed to create http client");
        return -1;
    }
    if (client->start() == 0)
    {
        LOG_INFO("HTTP CLIENT START SUCCESSFULLY");
    }
    else
    {
        LOG_ERROR("HTTP CLIENT START ERROR");
        return -2;
    }

    int port = 8888;
    CTcpServer *s = new CHttpServer(port);
    if (s == NULL)
    {
        LOG_ERROR("failed to create httpserver, bind port: %d", port);
        return -1;
    }

    s->set_next(client);

    if (s->start() == 0)
    {
        LOG_INFO("HTTP SERVER START SUCCESSFULLY");
    }
    else
    {
        LOG_ERROR("HTTP SERVER START ERROR");
        return -2;
    }

    client->join();
    s->join();

    return 0;
}
#else
#endif //MULTI_THREAD_VERSION
