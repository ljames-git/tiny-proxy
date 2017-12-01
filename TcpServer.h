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
    virtual int on_accept(int sock);
    virtual int on_data(int sock, char *buf, int size);
    virtual int on_close(int sock);
    virtual int on_error(int sock);
    virtual int on_write_done(int sock);

protected:
    virtual int do_close(int sock);

private:
    int m_port;
    int m_serv_sock;
};

#endif //__TCP_SERVER_H__
