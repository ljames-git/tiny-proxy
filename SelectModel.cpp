#include <stdio.h>
#include <string.h>

#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/ioctl.h>

#include "common.h"
#include "SelectModel.h"

CSelectModel *CSelectModel::m_inst = NULL;
pthread_mutex_t CSelectModel::m_mutex = PTHREAD_MUTEX_INITIALIZER;

CSelectModel::CSelectModel()
    :m_timeout(0)
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

int CSelectModel::set_read_fd(int fd, IRwComponent *component)
{
    if (fd < 0 || fd >= FD_SETSIZE || !component)
        return -1;

    FD_SET(fd, &m_read_set);

    if (m_components[fd] && component != m_components[fd])
        return -2;

    m_components[fd] = component;

    return 0;
}

int CSelectModel::set_write_fd(int fd, IRwComponent *component)
{
    if (fd < 0 || fd >= FD_SETSIZE || !component)
        return -1;

    FD_SET(fd, &m_write_set);

    if (m_components[fd] && component != m_components[fd])
        return -2;

    m_components[fd] = component;
    return 0;
}

int CSelectModel::clear_read_fd(int fd)
{
    if (fd < 0 || fd >= FD_SETSIZE)
        return -1;

    FD_CLR(fd, &m_read_set);

    if (m_components[fd])
        m_components[fd]->do_clean(fd);
    m_components[fd] = NULL;

    return 0;
}

int CSelectModel::clear_write_fd(int fd)
{
    if (fd < 0 || fd >= FD_SETSIZE)
        return -1;

    FD_CLR(fd, &m_write_set);

    if (m_components[fd])
        m_components[fd]->do_clean(fd);
    m_components[fd] = NULL;
    
    return 0;
}

int CSelectModel::clear_read_fd_set()
{
    FD_ZERO(&m_read_set);
}

int CSelectModel::clear_write_fd_set()
{
    FD_ZERO(&m_write_set);
}

int CSelectModel::set_timeout(int milli_sec)
{
    m_timeout = milli_sec;
    return 0;
}

int CSelectModel::start()
{

    struct timeval tv, *ptv;
    ptv = NULL;
    if (m_timeout > 0)
    {
        tv.tv_sec = m_timeout / 1000;
        tv.tv_usec = (m_timeout % 1000) * 1000;
        ptv = &tv;
    }

    int ret = 0;
    fd_set rset, wset;
    for (rset = m_read_set, wset = m_write_set;
            (ret = select(FD_SETSIZE, &rset, &wset, NULL, ptv)) >= 0;
            rset = m_read_set, wset = m_write_set)
    {
        if (ret == 0)
            continue;

        for (int fd = 0; fd < FD_SETSIZE; fd++)
        {
            if (FD_ISSET(fd, &rset))
            {
                IRwComponent *component = m_components[fd];
                if (component == NULL)
                {
                    char log_buf[1024];
                    snprintf(log_buf, sizeof(log_buf), "cannot find registered component, sock: %d", fd);
                    LOG_INFO(log_buf);

                    clear_read_fd(fd);
                    clear_write_fd(fd);
                    continue;
                }

                if (component->is_acceptable(fd))
                {
                    if (component->do_accept(fd) != 0)
                    {
                        clear_read_fd(fd);
                        continue;
                    }
                }
                else
                {
                    int to_read = 0;
                    ioctl(fd, FIONREAD, &to_read);
                    if (to_read <= 0)
                    {
                        char log_buf[1024];
                        snprintf(log_buf, sizeof(log_buf), "error sock on read or closed by peer, sock: %d", fd);
                        LOG_INFO(log_buf);

                        clear_read_fd(fd);
                        clear_write_fd(fd);
                        continue;
                    }

                    char *buf = new char[to_read];
                    if (buf == NULL)
                    {
                        char log_buf[1024];
                        snprintf(log_buf, sizeof(log_buf), "not enough memory, bytes: %d", to_read);
                        LOG_INFO(log_buf);
                        continue;
                    }

                    int nread = read(fd, buf, to_read);
                    if (nread == 0)
                    {
                        char log_buf[1024];
                        snprintf(log_buf, sizeof(log_buf), "closed by peer, sock: %d", fd);
                        LOG_INFO(log_buf);

                        clear_read_fd(fd);
                        clear_write_fd(fd);
                        delete []buf;

                        continue;
                    }
                    if (nread < 0)
                    {
                        if (errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR)
                        {
                            char log_buf[1024];
                            snprintf(log_buf, sizeof(log_buf), "error on read, sock: %d", fd);
                            LOG_INFO(log_buf);

                            clear_read_fd(fd);
                            clear_write_fd(fd);
                        }

                        delete []buf;
                        continue;
                    }

                    if (component->do_read(fd, buf, nread) != 0)
                    {
                        clear_read_fd(fd);
                        clear_write_fd(fd);
                    }
                    delete []buf;
                }
            }
            
            if (FD_ISSET(fd, &wset))
            {
            }
        }
    }
    return 0;
}
