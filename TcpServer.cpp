#include <stdio.h>
#include <string.h>

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/socket.h>

#include "common.h"
#include "TcpServer.h"
#include "SelectModel.h"


CTcpServer::CTcpServer(int port):
    m_port(port),
    m_serv_sock(0)
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

int CTcpServer::do_read(int sock, char *buf, int size)
{
    return 0;
}

int CTcpServer::do_write(int sock, char *buf, int size)
{
    return 0;
}

/*
int CTcpServer::select_model() 
{
    fd_set fd_read;
    FD_ZERO(&fd_read);
    FD_SET(m_serv_sock, &fd_read);
    
    for (fd_set fd_test = fd_read; select(FD_SETSIZE, &fd_test, NULL, NULL, NULL) >= 0; fd_test = fd_read)
    {
        for (int fd = 0; fd < FD_SETSIZE; fd++)
        {
            if (!FD_ISSET(fd, &fd_test))
                continue;

            if (fd == m_serv_sock)
            {
                struct sockaddr_in client_address;  
                socklen_t client_len = sizeof(client_address);
                int client_sock = accept(m_serv_sock, (struct sockaddr *)&client_address, &client_len);  
                if (client_sock <= 0)
                {
                    if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
                    {
                        continue;
                    }
                    break;
                }

                FD_SET(client_sock, &fd_read);
            }
            else
            {
                int nread = 0;
                ioctl(fd, FIONREAD, &nread);
                if (nread <= 0)
                {
                    close(fd);
                    FD_CLR(fd, &fd_read);
                    continue;
                }

                char *buf = new char[nread];
                if (buf == NULL)
                {
                    close(fd);
                    FD_CLR(fd, &fd_read);
                    continue;
                }

                int n = read(fd, buf, nread);
                if (n == 0)
                {
                    close(fd);
                    FD_CLR(fd, &fd_read);
                    continue;
                }
                if (n < 0)
                {
                    if (errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR)
                    {
                        close(fd);
                        FD_CLR(fd, &fd_read);
                    }
                    continue;
                }

                do_data(buf, n);
            }
        }
    }

    for (int fd = 0; fd < FD_SETSIZE; fd++)
    {
        if (FD_ISSET(fd, &fd_read))
        {
            close(fd);
        }
    }

    return 0;
}
*/

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

    IMultiPlexer *multi_plexer = CSelectModel::instance();
    if (!multi_plexer || multi_plexer->set_read_fd(m_serv_sock, this) != 0)
        return -1;

    return 0;
}
