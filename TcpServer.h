#ifndef __TCP_SERVER_H__
#define __TCP_SERVER_H__

#include "MultiPlexer.h"
#include "RwComponent.h"

class CTcpServer: public IRwComponent
{
    public:
        CTcpServer(int port);
        ~CTcpServer();
        int start();

    protected:
        virtual int do_read(int sock, char *buf, int size);
        virtual int do_write(int sock, char *buf, int size);

    private:
        int m_port;
        int m_serv_sock;
};

#endif //__TCP_SERVER_H__
