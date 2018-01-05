#include <string>
#include <stdlib.h>
#include <string.h>

#include "msg.h"
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

int CHttpServer::send_404(http_task_t *task)
{
    if (!task)
    {
        LOG_WARN("CHttpServer::send_404 task is null");
        return -1;
    }

    const char *content = "HTTP/1.1 404 Not Found\r\nServer: Tiny-Proxy/0.1\r\n\r\n";
    int len = strlen(content);
    get_model()->write(task->sock, content, len, this);

    return 0;
}

int CHttpServer::parse_req_header(http_task_t *task)
{
    if (task == NULL) 
    {
        LOG_WARN("CHttpServer::parse_req_header task null");
        return -1;
    }

    if (task->body_offset > 0)
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
                // header end
                task->body_offset = ++(task->header_size);
                return 1;
            }

            if (task->req.m_header.get_method() == HTTP_METHOD_NONE)
            {
                // parse first line
                // [method] [path] HTTP/x.x

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
                {
                    LOG_WARN("CHttpServer::parse_req_header unsupported method: %s", method);
                    return -1;
                }

                // parse uri/path
                char *r = CStringUtil::trim(t + 1);
                for (t = r; *t && *t != ' '; t++);
                *t = 0;
                char *uri = CStringUtil::trim(r);
                task->req.m_header.set_uri(uri);
            }
            else
            {
                // parse other header items
                char *t = q;
                for (; *t && *t != ':'; t++);
                if (*t == ':')
                {
                    *t = 0;
                    const char *key = CStringUtil::trim(q);
                    const char *value = CStringUtil::trim(t + 1);
                    task->req.m_header.set_value(key, value);
                    
                    // get content length
                    if (strcmp(key, "Content-Length") == 0)
                        task->req.m_header.set_content_length(atoi(value));
                }
            }

            q = p + 1;
        }
    }

    if (task->header_size >= HTTP_HEADER_LEN)
    {
        // header is too long
        LOG_WARN("CHttpServer::parse_req_header header is too long, size: %d, uri: %s", task->header_size, task->req.m_header.get_uri());
        LOG_WARN("%s", task->header_buf);
        return -1;
    }

    return 0;
}

int CHttpServer::on_data(int sock, char *buf, int size)
{
    // find task
    http_task_t *task = NULL;
    if (m_task_map.find(sock) == m_task_map.end())
    {
        task = new http_task_t(sock);
        if (task == NULL)
        {
            LOG_WARN("CHttpServer::on_data task is null");
            return -1;
        }

        m_task_map[sock] = task;
    }
    task = m_task_map[sock];

    // ignore data when request done
    if (task->req_done)
        return 0;

    if (task->body_offset == 0)
    {
        // receiving header
        int copy_size = size > HTTP_HEADER_LEN - task->header_buf_size ? HTTP_HEADER_LEN - task->header_buf_size : size;
        memcpy(task->header_buf + task->header_size, buf, copy_size);
        task->header_buf_size += copy_size;
    }

    // parse header 
    int state = parse_req_header(task);

    // return negtive means error occurred
    if (state < 0)
    {
        LOG_WARN("CHttpServer::on_data parse request error");
        return -1;
    }

    // positive means header done
    if (state > 0)
    {
        int content_length = task->req.m_header.get_content_length();
        if (content_length < 0)
        {
            LOG_WARN("CHttpServer::on_data content length invalid: %d", content_length);
            return -1;
        }

        if (!task->body)
        {
            task->body = new char[content_length];
            if (!task->body)
            {
                LOG_WARN("CHttpServer::on_data alloc body memory error");
                return -1;
            }

            // copy body data from header buf if any
            int copy_size = task->header_buf_size - task->body_offset;
            if (copy_size > 0)
            {
                memcpy(task->body, task->header_buf + task->body_offset, copy_size);
                task->body_size += copy_size;
            }
        }
        else
        {
            int copy_size = size;
            if (task->body_size + copy_size >= content_length)
                copy_size = content_length - task->body_size;

            memcpy(task->body, buf, copy_size);
            task->body_size += copy_size;
        }

        if (task->body_size >= content_length)
        {
            // request done
            task->req_done = 1;
            task_done(task);
        }
    }

    return 0;
}

int CHttpServer::do_close(int sock)
{
    task_map_t::iterator it = m_task_map.find(sock);
    if (it != m_task_map.end())
    {
        delete it->second;
        m_task_map.erase(it);
    }

    return CTcpServer::do_close(sock);
}

int CHttpServer::task_done(http_task_t *task)
{
    if (!task)
    {
        LOG_WARN("CHttpServer::task_done task is null");
        return -1;
    }

    LOG_STAT("task done: %s", task->req.m_header.get_uri());

    msg_t *msg = new msg_t;
    msg->server = this;
    msg->task = task;
    get_next()->m_msg_queue.enqueue(msg);

    LOG_STAT("enqueue: %s", task->req.m_header.get_uri());

    return 0;
}
