#ifndef __TCP_SERVER_H__
#define __TCP_SERVER_H__

#include "PipeLine.h"
#include "MultiPlexer.h"
#include "RwComponent.h"

class CTcpServer: public IRwComponent
{
public:
    CTcpServer();
    CTcpServer(int port);
    CTcpServer(int port, IMultiPlexer *multi_plexer);
    ~CTcpServer();

    IMultiPlexer *get_multi_plexer();
    int set_multi_plexer(IMultiPlexer *multi_plexer);
    virtual int start(IMultiPlexer *multi_plexer = NULL);

public:
    // implementaions of IRwComponent interface
    virtual bool is_acceptable(int sock);
    virtual int on_accept(int sock);
    virtual int on_data(int sock, char *buf, int size);
    virtual int on_close(int sock);
    virtual int on_error(int sock);
    virtual int do_write(int sock);
    virtual void *get_message(int sock);

//protected:
    virtual int do_close(int sock);

private:
    int m_port;
    int m_serv_sock;
    IMultiPlexer *m_multi_plexer;
};

#endif //__TCP_SERVER_H__
