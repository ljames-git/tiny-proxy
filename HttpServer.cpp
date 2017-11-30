#include <string.h>

#include "common.h"
#include "StringUtil.h"
#include "HttpServer.h"

CHttpServer::CHttpServer():
    CTcpServer(80)
{
}

CHttpServer::CHttpServer(int port):
    CTcpServer(port)
{
}

CHttpServer::~CHttpServer()
{
}

int CHttpServer::parse_req_header(http_task_t *task)
{
    if (task == NULL) 
        return -1;
    if (task->body != NULL)
        return 1;

    for (char *p = task->header_buf + task->header_size, *q = p;
            task->header_size < HTTP_HEADER_LEN;
            p++, task->header_size++)
    {
        if (*p == '\n' && p != task->header_buf && *(p - 1) == '\r')
        {
            *(p - 1) = 0;
            if (strlen(q) == 0)
            {
                {
                    char log_buf[1024];
                    snprintf(log_buf, sizeof(log_buf), "%d", task->req.m_header.get_method());
                    LOG_INFO(log_buf);
                    LOG_INFO(task->req.m_header.get_uri());

                    int size = 0;
                    char **p = task->req.m_header.get_keys(&size);
                    for (char **q = p; q - p < size; q++)
                    {
                        char log_buf[1024];
                        snprintf(log_buf, sizeof(log_buf), "%s:%s", *q, task->req.m_header.get_value(*q));
                        LOG_INFO(log_buf);
                    }

                }
                task->body = p;
                return 0;
            }

            if (task->req.m_header.get_method() == HTTP_METHOD_NONE)
            {
                char *t = q;

                // parse method
                for (; *t && *t != ' '; t++);
                if (!*t)
                    return -1;
                *t = 0;
                char *method = CStringUtil::trim(q);
                if (strcmp(method, "OPTIONS") == 0)
                    task->req.m_header.set_method(HTTP_METHOD_OPTIONS);
                else if (strcmp(method, "GET") == 0)
                    task->req.m_header.set_method(HTTP_METHOD_GET);
                else if (strcmp(method, "POST") == 0)
                    task->req.m_header.set_method(HTTP_METHOD_POST);
                else if (strcmp(method, "CONNECT") == 0)
                    task->req.m_header.set_method(HTTP_METHOD_CONNECT);
                else
                    return -1;

                // parse uri
                char *r = CStringUtil::trim(t + 1);
                for (t = r; *t && *t != ' '; t++);
                *t = 0;
                char *uri = CStringUtil::trim(r);
                task->req.m_header.set_uri(uri);
            }
            else
            {
                char *t = q;
                for (; *t && *t != ':'; t++);
                if (*t == ':')
                {
                    *t = 0;
                    task->req.m_header.set_value(CStringUtil::trim(q), CStringUtil::trim(t + 1));
                }
            }

            q = p + 1;
        }
    }

    return 0;
}

int CHttpServer::on_data(int sock, char *buf, int size)
{
    http_task_t *task = NULL;
    if (m_task_map.find(sock) == m_task_map.end())
    {
        task = new http_task_t(sock);
        if (task == NULL)
            return -1;

        m_task_map[sock] = task;
    }
    task = m_task_map[sock];

    int copy_size = size > HTTP_HEADER_LEN ? HTTP_HEADER_LEN : size;
    memcpy(task->header_buf, buf, copy_size);
    if (parse_req_header(task) < 0)
        return -1;

    return 0;
}
