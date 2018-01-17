#include <signal.h>
#include "common.h"
#include "PipeLine.h"
#include "TcpServer.h"
#include "HttpServer.h"
#include "HttpClient.h"
#include "SelectModel.h"

int main(int argc, char ** argv)
{
    signal(SIGPIPE, SIG_IGN);

    IMultiPlexer *multi_plexer = CSelectModel::instance();

    int port = 8888;
    CTcpServer *server = new CHttpServer(port, multi_plexer);
    if (server == NULL)
    {
        LOG_ERROR("failed to create server, bind port: %d", port);
        return -1;
    }
    if (server->start() == 0)
    {
        LOG_INFO("SERVER BIND SUCCESSFULLY");
    }
    else
    {
        LOG_ERROR("SERVER BIND ERROR");
        return -2;
    }

    CPipeLine *client_pl = new CPipeLine(8);
    client_pl->start(new CHttpClient);

    CPipeLine *server_pl = new CPipeLine;
    server_pl->set_next(client_pl);
    server_pl->start(multi_plexer);

    LOG_INFO("START SUCCESSFULLY");

    server_pl->join();
    client_pl->join();

    return 0;
}
