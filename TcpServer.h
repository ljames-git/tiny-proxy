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

        virtual IMultiPlexer* get_model();

    public:
        // implementaions of IRwComponent interface
        virtual bool is_acceptable(int sock);
        virtual int do_accept(int sock);
        virtual int do_clean(int sock);
        virtual int do_read(int sock, char *buf, int size);
        virtual int do_write(int sock, char *buf, int size);

    private:
        int m_port;
        int m_serv_sock;
};

#endif //__TCP_SERVER_H__
