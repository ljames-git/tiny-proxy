#include <string.h>

#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "common.h"
#include "SelectModel.h"

CSelectModel *CSelectModel::m_inst = NULL;
pthread_mutex_t CSelectModel::m_mutex = PTHREAD_MUTEX_INITIALIZER;

CSelectModel::CSelectModel() :
    m_timeout(0),
    m_pipe_line(NULL)
{
    FD_ZERO(&m_read_set);
    FD_ZERO(&m_write_set);
    memset(m_components, 0, sizeof(m_components));
}

CSelectModel::~CSelectModel()
{
}

CSelectModel *CSelectModel::instance()
{
    if (m_inst == NULL)
    {
        pthread_mutex_lock(&m_mutex);
        if (m_inst == NULL)
        {
            m_inst = new CSelectModel();
        }
        pthread_mutex_unlock(&m_mutex);
    }

    return m_inst;
}

int CSelectModel::set_pipe_line(CPipeLine *pipe_line)
{
    if (pipe_line == NULL)
    {
        LOG_WARN("fail to set pipe line: NULL");
        return -1;
    }

    m_pipe_line = pipe_line;
    return 0;
}

int CSelectModel::set_read_fd(int fd, IRwComponent *component)
{
    if (fd < 0 || fd >= FD_SETSIZE || !component)
    {
        LOG_WARN("[CSelectModel::set_read_fd] failed to set read fd: %d", fd);
        return -1;
    }

    FD_SET(fd, &m_read_set);

    if (m_components[fd] && component != m_components[fd])
    {
        LOG_WARN("failed to set read fd, has component, fd: %d", fd);
        return -2;
    }

    m_components[fd] = component;
    return 0;
}

int CSelectModel::set_write_fd(int fd, IRwComponent *component)
{
    if (fd < 0 || fd >= FD_SETSIZE || !component)
    {
        LOG_WARN("failed to set write fd: %d", fd);
        return -1;
    }

    FD_SET(fd, &m_write_set);

    if (m_components[fd] && component != m_components[fd])
    {
        LOG_WARN("failed to set read fd, has component, fd: %d", fd);
        return -2;
    }

    m_components[fd] = component;
    return 0;
}

int CSelectModel::clear_fd(int fd)
{
    if (fd < 0 || fd >= FD_SETSIZE)
    {
        LOG_WARN("failed to set clear fd: %d", fd);
        return -1;
    }

    FD_CLR(fd, &m_read_set);
    FD_CLR(fd, &m_write_set);
    m_components[fd] = NULL;

    return 0;
}

int CSelectModel::set_timeout(int milli_sec)
{
    m_timeout = milli_sec;
    return 0;
}

int CSelectModel::run()
{
    if (m_pipe_line)
    {
        LOG_INFO("PIPE LINE MODE ON");
    }
    else
    {
        LOG_INFO("PIPE LINE MODE OFF");
    }

    struct timeval ctv, tv, *ptv;
    ptv = NULL;
    if (m_timeout > 0)
    {
        tv.tv_sec = m_timeout / 1000;
        tv.tv_usec = (m_timeout % 1000) * 1000;
        ctv = tv;
        ptv = &tv;
    }

    for (;;)
    {
        int ret = 0; 
        fd_set rset, wset;
        for (rset = m_read_set, wset = m_write_set;
                (ret = select(FD_SETSIZE, &rset, &wset, NULL, ptv)) >= 0;
                rset = m_read_set, wset = m_write_set, tv = ctv)
        {
            if (ret == 0)
            {
                continue;
            }

            for (int fd = 0; fd < FD_SETSIZE; fd++)
            {
                if (FD_ISSET(fd, &rset))
                {
                    IRwComponent *component = m_components[fd];
                    if (component == NULL)
                    {
                        // fd might be closed by other threads
                        close(fd);
                        FD_CLR(fd, &m_read_set);
                        continue;
                    }

                    if (component->is_acceptable(fd))
                    {
                        struct sockaddr_in client_address;  
                        socklen_t client_len = sizeof(client_address);
                        int client_sock = accept(fd, (struct sockaddr *)&client_address, &client_len);  
                        if (client_sock <= 0)
                        {
                            if (errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR)
                            {
                                LOG_DEBUG("accept error, sock: %d", fd);
                                component->on_error(fd);
                            }
                            continue;
                        }

                        if (component->on_accept(client_sock) != 0)
                        {
                            LOG_WARN("componet on_accept error");
                            component->on_error(client_sock);
                            continue;
                        }
                    }
                    else
                    {
                        int to_read = 0;
                        ioctl(fd, FIONREAD, &to_read);
                        if (to_read == 0)
                        {
                            LOG_DEBUG("sock closed by peer, sock: %d", fd);
                            component->on_close(fd);
                            continue;
                        }
                        if (to_read < 0)
                        {
                            LOG_WARN("read error on sock: %d", fd);
                            component->on_error(fd);
                            continue;
                        }

                        char *buf = new char[to_read];
                        if (buf == NULL)
                        {
                            LOG_WARN("not enough memory, bytes: %d", to_read);
                            continue;
                        }

                        int nread = read(fd, buf, to_read);
                        if (nread == 0)
                        {
                            delete []buf;
                            LOG_DEBUG("sock closed by peer, sock: %d", fd);
                            component->on_close(fd);

                            continue;
                        }
                        if (nread < 0)
                        {
                            if (errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR)
                            {
                                LOG_WARN("read error on sock: %d", fd);
                                component->on_error(fd);
                            }

                            delete []buf;
                            continue;
                        }

                        int ret = component->on_data(fd, buf, nread);
                        if (ret < 0)
                        {
                            // error
                            LOG_WARN("process data error, sock: %d", fd);
                            component->on_error(fd);
                        }
                        if (ret == 0)
                        {
                            // read done
                            void *msg = NULL;
                            if (m_pipe_line && m_pipe_line->get_next() && (msg = component->get_message(fd)))
                            {
                                m_pipe_line->get_next()->m_msg_queue.enqueue(msg);
                            }
                        }
                        if (ret > 0)
                        {
                            // some date to read next time
                        }

                        delete []buf;
                    }
                }

                if (FD_ISSET(fd, &wset))
                {
                    IRwComponent *component = m_components[fd];
                    if (component == NULL)
                    {
                        // fd might be closed by other threads
                        close(fd);
                        FD_CLR(fd, &m_write_set);
                        continue;
                    }

                    int r = component->do_write(fd);
                    if (r == 0)
                    {
                        // write done
                        // close
                        component->on_close(fd);
                    }
                    else if (r > 0)
                    {
                        // some date not be written
                        // write next time
                    }
                    else
                    {
                        // error
                        // close
                        component->on_error(fd);
                    }
                }
            }
        }

        LOG_WARN("select error, errno: %d", errno);
    }
    return 0;
}
