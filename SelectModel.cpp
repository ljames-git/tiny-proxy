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
    {
        LOG_WARN("failed to set read fd: %d", fd);
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

int CSelectModel::write(int fd, const char *buf, int size, IRwComponent *component)
{
    if (fd < 0 || fd >= FD_SETSIZE || buf == NULL || size <= 0 || component == NULL)
    {
        LOG_WARN("write failed, fd: %d, size: %d", fd, size);
        return -1;
    }

    if (m_rw_objects[fd])
    {
        LOG_WARN("write failed, has object, fd: %d", fd);
        return -1;
    }

    CRwObject *obj = CRwObject::create_object(size);
    if (obj == NULL)
    {
        LOG_WARN("write failed, failed to create object");
        return -1;
    }

    if (m_components[fd] && component != m_components[fd])
    {
        LOG_WARN("write failed, component error");
        delete obj;
        return -2;
    }

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
    {
        LOG_WARN("failed to write, object null");
        return -1;
    }

    int ret = ::write(fd, obj->m_buffer + obj->m_done_size, obj->m_left_size);
    if (ret < 0)
    {
        if (errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR)
        {
            LOG_WARN("failed to write, errno: %d", errno);
            return -1;
        }
        return 1;
    }

    obj->m_done_size += ret;
    obj->m_left_size -= ret;
    if (obj->m_left_size <= 0)
        return 0;

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
        {
            LOG_DEBUG("select return 0");
            continue;
        }

        for (int fd = 0; fd < FD_SETSIZE; fd++)
        {
            if (FD_ISSET(fd, &rset))
            {
                IRwComponent *component = m_components[fd];
                if (component == NULL)
                {
                    LOG_WARN("cannot find registered component, sock: %d", fd);
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

                    if (component->on_data(fd, buf, nread) != 0)
                    {
                        LOG_WARN("process data error, sock: %d", fd);
                        component->on_error(fd);
                    }
                    delete []buf;
                }
            }
            
            if (FD_ISSET(fd, &wset))
            {
                IRwComponent *component = m_components[fd];
                if (component == NULL)
                {
                    LOG_WARN("cannot find registered component, sock: %d", fd);
                    close(fd);
                    continue;
                }

                int ret = do_write(fd);
                if (ret < 0)
                {
                    LOG_WARN("do write error, ret: %d, sock: %d", ret, fd);
                    component->on_error(fd);
                }
                if (ret == 0)
                {
                    LOG_DEBUG("write done, sock: %d", fd);
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
