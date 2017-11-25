#include <stdio.h>
#include <string.h>

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "common.h"
#include "TcpServer.h"
#include "SelectModel.h"


CTcpServer::CTcpServer(int port):
    m_port(port),
    m_serv_sock(-1)
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

IMultiPlexer* CTcpServer::get_model()
{
    return CSelectModel::instance();
}

bool CTcpServer::is_acceptable(int sock)
{
    return sock == m_serv_sock;
}

int CTcpServer::do_accept(int sock)
{
    if (sock != m_serv_sock)
        return -1;

    struct sockaddr_in client_address;  
    socklen_t client_len = sizeof(client_address);
    int client_sock = accept(sock, (struct sockaddr *)&client_address, &client_len);  
    if (client_sock <= 0)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
        {
            return 0;
        }

        char log_buf[1024];
        snprintf(log_buf, sizeof(log_buf), "accept error, sock: %d", sock);
        LOG_INFO(log_buf);

        return -1;
    }

    int flags = fcntl(client_sock, F_GETFL, 0);
    fcntl(client_sock, F_SETFL, flags|O_NONBLOCK);

    IMultiPlexer *multi_plexer = get_model();
    if (!multi_plexer || multi_plexer->set_read_fd(client_sock, this) != 0)
        return -1;

    return 0;
}

int CTcpServer::do_clean(int sock)
{
    char log_buf[1024];
    snprintf(log_buf, sizeof(log_buf), "close sock: %d", sock);
    LOG_INFO(log_buf);

    close(sock);
    return 0;
}

int CTcpServer::do_read(int sock, char *buf, int size)
{
    LOG_INFO(buf);
    return 0;
}

int CTcpServer::do_write(int sock, char *buf, int size)
{
    return 0;
}

int CTcpServer::start()
{
    if (m_port <= 0)
    {
        LOG_ERROR("invalid port");
        return -1;
    }

    // socket
    ASSIGN_AND_ERROR_ON_NEG(m_serv_sock, socket(AF_INET, SOCK_STREAM, 0));

    // nonblock socket
    int flags = fcntl(m_serv_sock, F_GETFL, 0);
    fcntl(m_serv_sock, F_SETFL, flags|O_NONBLOCK);

    // bind
    int r = 0;
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(struct sockaddr_in));  
    serv_addr.sin_family = AF_INET;  
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);  
    serv_addr.sin_port = htons(m_port);  
    ERROR_ON_NEG(bind(m_serv_sock, (struct sockaddr*)&serv_addr, sizeof(struct sockaddr_in))); 

    // listen
    ERROR_ON_NEG(listen(m_serv_sock, 1024));

    IMultiPlexer *multi_plexer = get_model();
    if (!multi_plexer || multi_plexer->set_read_fd(m_serv_sock, this) != 0)
        return -1;

    return 0;
}
