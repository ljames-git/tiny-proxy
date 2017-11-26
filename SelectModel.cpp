#include <stdio.h>
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

CSelectModel::CSelectModel()
    :m_timeout(0)
{
    FD_ZERO(&m_read_set);
    FD_ZERO(&m_write_set);
    memset(m_components, 0, sizeof(m_components));
    memset(m_rw_objects, 0, sizeof(m_rw_objects));
}

CSelectModel::~CSelectModel()
{
    for (int i = 0; i < FD_SETSIZE; i++)
    {
        if (m_rw_objects[i])
        {
            delete m_rw_objects[i];
            m_rw_objects[i] = NULL;
        }
    }
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

int CSelectModel::clear_fd(int fd)
{
    if (fd < 0 || fd >= FD_SETSIZE)
        return -1;

    FD_CLR(fd, &m_read_set);
    FD_CLR(fd, &m_write_set);
    m_components[fd] = NULL;

    if (m_rw_objects[fd])
    {
        delete m_rw_objects[fd];
        m_rw_objects[fd] = NULL;
    }

    return 0;
}

int CSelectModel::set_timeout(int milli_sec)
{
    m_timeout = milli_sec;
    return 0;
}

int CSelectModel::write(int fd, char *buf, int size, IRwComponent *component)
{
    if (fd < 0 || fd >= FD_SETSIZE || buf == NULL || size <= 0 || component == NULL)
        return -1;

    if (m_rw_objects[fd])
    {
        return -1;
    }

    CRwObject *obj = CRwObject::create_object(size);
    if (obj == NULL)
        return -1;

    if (m_components[fd] && component != m_components[fd])
        return -2;

    m_components[fd] = component;
    memcpy(obj->m_buffer, buf, size);
    m_rw_objects[fd] = obj;

    FD_SET(fd, &m_write_set);

    return 0;
}

int CSelectModel::do_write(int fd)
{
    CRwObject *obj = m_rw_objects[fd];
    if (obj == NULL)
        return -1;

    int ret = ::write(fd, obj->m_buffer + obj->m_done_size, obj->m_left_size);
    if (ret < 0)
    {
        if (errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR)
        {
            return -1;
        }
        return 1;
    }

    obj->m_done_size += ret;
    obj->m_left_size -= ret;
    if (obj->m_left_size <= 0)
    {
        return 0;
    }
    return 1;
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

    int ret = 0; fd_set rset, wset;
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

                    close(fd);
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
                            char log_buf[1024];
                            snprintf(log_buf, sizeof(log_buf), "accept error, sock: %d", fd);
                            LOG_INFO(log_buf);

                            component->on_error(fd);
                        }
                        continue;
                    }

                    if (component->on_accept(client_sock) != 0)
                    {
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
                        char log_buf[1024];
                        snprintf(log_buf, sizeof(log_buf), "sock closed by peer, sock: %d", fd);
                        LOG_INFO(log_buf);

                        component->on_close(fd);
                        continue;
                    }
                    if (to_read < 0)
                    {
                        char log_buf[1024];
                        snprintf(log_buf, sizeof(log_buf), "read error on sock: %d", fd);
                        LOG_INFO(log_buf);

                        component->on_error(fd);
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
                        char log_buf[1024]; snprintf(log_buf, sizeof(log_buf), "sock closed by peer, sock: %d", fd);
                        LOG_INFO(log_buf);

                        delete []buf;
                        component->on_close(fd);

                        continue;
                    }
                    if (nread < 0)
                    {
                        if (errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR)
                        {
                            char log_buf[1024];
                            snprintf(log_buf, sizeof(log_buf), "read error on sock: %d", fd);
                            LOG_INFO(log_buf);

                            component->on_error(fd);
                        }

                        delete []buf;
                        continue;
                    }

                    if (component->on_data(fd, buf, nread) != 0)
                        component->on_error(fd);
                    delete []buf;
                }
            }
            
            if (FD_ISSET(fd, &wset))
            {
                IRwComponent *component = m_components[fd];
                if (component == NULL)
                {
                    char log_buf[1024];
                    snprintf(log_buf, sizeof(log_buf), "cannot find registered component, sock: %d", fd);
                    LOG_INFO(log_buf);

                    close(fd);
                    continue;
                }

                int ret = do_write(fd);
                if (ret < 0)
                {
                    component->on_error(fd);
                }
                if (ret == 0)
                {
                    delete m_rw_objects[fd];
                    m_rw_objects[fd] = NULL;
                    FD_CLR(fd, &m_write_set);
                    component->on_write_done(fd);
                }
            }
        }
    }
    return 0;
}
