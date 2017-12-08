#include <vector>
#include <string>
#include <stdlib.h>
#include <string.h>

#include "curl/curl.h"

#include "common.h"
#include "StringUtil.h"
#include "HttpServer.h"


static size_t OnWriteData(void* buffer, size_t size, size_t nmemb, void* lpVoid)  
{  
    std::vector<char>* wbuf = dynamic_cast<std::vector<char>*>((std::vector<char> *)lpVoid);  
    if( NULL == wbuf || NULL == buffer )  
    {  
        return -1;  
    }  

    char* pData = (char*)buffer;  
    wbuf->insert(wbuf->end(), pData, pData + size * nmemb);
    return nmemb;  
}  

int send_req(CHttpServer *server, http_task_t *task)
{
    CURLcode res;
    CURL* curl = curl_easy_init();
    if(NULL == curl)
        return -1;

    std::vector<char> wbuf;
    std::string url = "";
    CHttpHeader& header = task->req.m_header;
    url += header.get_uri();
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

    struct curl_slist *list = NULL;
    int size = 0;
    char **p = header.get_keys(&size);
    for (char **q = p; q - p < size; q++)
    {
        char buf[1024];
        snprintf(buf, sizeof(buf), "%s:%s", *q, header.get_value(*q));
        list = curl_slist_append(list, buf);
    }
    delete []p;

    if (list)
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);

    switch(header.get_method())
    {
    case HTTP_METHOD_POST:
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, task->body_size);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, task->body);
        break;
    default:
        break;
    }
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnWriteData);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&wbuf);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
    curl_easy_setopt(curl, CURLOPT_HEADER, 1);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 3);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3);
    curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_0);
    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    server->get_model()->write(task->sock, wbuf.data(), wbuf.size(), server);
    return 0;
}

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
                    return -1;

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
            return -1;

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
        return -1;

    // positive means header done
    if (state > 0)
    {
        int content_length = task->req.m_header.get_content_length();
        if (content_length < 0)
            return -1;

        if (!task->body)
        {
            task->body = new char[content_length];
            if (!task->body)
                return -1;

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
        return -1;

    send_req(this, task);
    return 0;
}
