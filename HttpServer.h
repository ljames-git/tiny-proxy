#ifndef __HTTP_SERVER_H__
#define __HTTP_SERVER_H__

#include "TcpServer.h"

class CHttpServer: public CTcpServer
{
public:
    CHttpServer();
    CHttpServer(int port);
    ~CHttpServer();

public:
    virtual int on_data(int sock, char *buf, int size);
};

#endif //__HTTP_SERVER_H__
