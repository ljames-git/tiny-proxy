#include "HttpServer.h"

CHttpServer::CHttpServer():
    CTcpServer(80)
{
}

CHttpServer::CHttpServer(int port):
    CTcpServer(port)
{
}

CHttpServer::~CHttpServer()
{
}

int CHttpServer::on_data(int sock, char *buf, int size)
{
    return CTcpServer::on_data(sock, buf, size);
}
