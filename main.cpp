#include <signal.h>
#include "common.h"
#include "PipeLine.h"
#include "TcpServer.h"
#include "HttpServer.h"
#include "HttpClient.h"
#include "SelectModel.h"

#ifndef MULTI_THREAD_VERSION
int main(int argc, char ** argv)
{
    signal(SIGPIPE, SIG_IGN);

    IMultiPlexer *multi_plexer = CSelectModel::instance();

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
    CTcpServer *s = new CHttpServer(port, multi_plexer);
    if (s == NULL)
    {
        LOG_ERROR("failed to create server, bind port: %d", port);
        return -1;
    }
    if (s->start() == 0)
    {
        LOG_INFO("SERVER START SUCCESSFULLY");
    }
    else
    {
        LOG_ERROR("SERVER START ERROR");
        return -2;
    }

    CPipeLine *server_thread = new CPipeLine;
    server_thread->start(multi_plexer);

    client->join();
    server_thread->join();

    return 0;
}
#else
#endif //MULTI_THREAD_VERSION
