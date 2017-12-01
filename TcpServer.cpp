#include <string.h>

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

int CTcpServer::on_accept(int sock)
{
    int flags = fcntl(sock, F_GETFL, 0);
    fcntl(sock, F_SETFL, flags|O_NONBLOCK);

    IMultiPlexer *multi_plexer = get_model();
    if (!multi_plexer || multi_plexer->set_read_fd(sock, this) != 0)
        return -1;

    return 0;
}

int CTcpServer::on_data(int sock, char *buf, int size)
{
    if (buf == NULL || size <= 0)
        return -1;

    LOG_INFO(buf);
    close(sock);
    IMultiPlexer *multi_plexer = get_model();
    if (!multi_plexer || multi_plexer->clear_fd(sock) != 0)
        return -1;

    return 0;
}

int CTcpServer::on_close(int sock)
{
    return do_close(sock);
}

int CTcpServer::on_error(int sock)
{
    return do_close(sock);
}

int CTcpServer::on_write_done(int sock)
{
    return 0;
}

int CTcpServer::do_close(int sock)
{
    char log_buf[1024];
    snprintf(log_buf, sizeof(log_buf), "close sock: %d", sock);
    LOG_INFO(log_buf);

    if (sock == m_serv_sock)
    {
        close(sock);
        return 0;
    }

    close(sock);
    IMultiPlexer *multi_plexer = get_model();
    if (!multi_plexer || multi_plexer->clear_fd(sock) != 0)
        return -1;

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

    IMultiPlexer *multi_plexer = get_model();
    if (!multi_plexer || multi_plexer->set_read_fd(m_serv_sock, this) != 0)
        return -1;

    return 0;
}
