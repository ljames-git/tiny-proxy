#include <errno.h>
#include <string.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "common.h"
#include "TcpServer.h"
#include "SelectModel.h"

CTcpServer::CTcpServer():
    m_port(-1),
    m_serv_sock(-1),
    m_multi_plexer(NULL)
{
}

CTcpServer::CTcpServer(int port):
    m_port(port),
    m_serv_sock(-1),
    m_multi_plexer(NULL)
{
}

CTcpServer::CTcpServer(int port, IMultiPlexer *multi_plexer):
    m_port(port),
    m_serv_sock(-1),
    m_multi_plexer(multi_plexer)
{
}

CTcpServer::~CTcpServer()
{
    if (m_serv_sock > 0)
    {
        close(m_serv_sock);
        m_serv_sock = 0;
    }
}

IMultiPlexer* CTcpServer::get_multi_plexer()
{
    return m_multi_plexer;
}

int CTcpServer::set_multi_plexer(IMultiPlexer *multi_plexer)
{
    if (multi_plexer == NULL)
        return -1;

    m_multi_plexer = multi_plexer;
    return 0;
}

bool CTcpServer::is_acceptable(int sock)
{
    return sock == m_serv_sock;
}

int CTcpServer::on_accept(int sock)
{
    int flags = fcntl(sock, F_GETFL, 0);
    fcntl(sock, F_SETFL, flags|O_NONBLOCK);

    if (m_multi_plexer->set_read_fd(sock, this) != 0)
        return -1;

    return 0;
}

int CTcpServer::on_data(int sock, char *buf, int size)
{
    if (buf == NULL || size <= 0)
        return -1;

    return 1;
}

int CTcpServer::on_close(int sock)
{
    return do_close(sock);
}

int CTcpServer::on_error(int sock)
{
    return do_close(sock);
}

int CTcpServer::do_write(int sock)
{
    return 0;
}

int CTcpServer::do_close(int sock)
{
    LOG_DEBUG("close sock: %d", sock);

    if (sock == m_serv_sock)
    {
        close(sock);
        return 0;
    }

    close(sock);
    if (m_multi_plexer->clear_fd(sock) != 0)
        return -1;
    return 0;
}

void * CTcpServer::get_message(int sock)
{
    return NULL;
}

int CTcpServer::start(IMultiPlexer *multi_plexer)
{
    if (m_port <= 0)
    {
        LOG_ERROR("invalid port");
        return -1;
    }

    set_multi_plexer(multi_plexer);
    if (!m_multi_plexer)
    {
        LOG_ERROR("multi_plexer null");
        return -1;
    }

    // socket
    ASSIGN_AND_ERROR_ON_NEG(m_serv_sock, socket(AF_INET, SOCK_STREAM, 0));

    // nonblock socket
    int flags = fcntl(m_serv_sock, F_GETFL, 0);
    fcntl(m_serv_sock, F_SETFL, flags|O_NONBLOCK);

    // bind
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(struct sockaddr_in));  
    serv_addr.sin_family = AF_INET;  
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);  
    serv_addr.sin_port = htons(m_port);  

    int opt = 1;
    setsockopt(m_serv_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    ERROR_ON_NEG(bind(m_serv_sock, (struct sockaddr*)&serv_addr, sizeof(struct sockaddr_in))); 

    // listen
    ERROR_ON_NEG(listen(m_serv_sock, 1024));

    if (multi_plexer->set_read_fd(m_serv_sock, this) != 0) //|| multi_plexer->set_timeout(50) != 0)
        return -1;

    /*
    for (;;)
    {
        multi_plexer->run();
        LOG_WARN("select error, errno: %d", errno);
    }
    */

    return 0;
}
