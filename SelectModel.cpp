#include <string.h>
#include <pthread.h>
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
    m_components[fd] = NULL;
    return 0;
}

int CSelectModel::clear_write_fd(int fd)
{
    if (fd < 0 || fd >= FD_SETSIZE)
        return -1;

    FD_CLR(fd, &m_write_set);
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
    fd_set rset, wset;
    int ret = 0;
    for (rset = m_read_set, wset = m_write_set;
            (ret = select(FD_SETSIZE, &rset, &wset, NULL, NULL)) >= 0;
            rset = m_read_set, wset = m_write_set)
    {
    }
    return 0;
}
