#include <vector>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include "curl/curl.h"

#include "msg.h"
#include "common.h"
#include "HttpClient.h"

static int write_with_timeout(int sock, char *buffer, int size, int timeout = 0)
{
    if (sock <= 0 || !buffer || size <= 0)
    {
        LOG_WARN("error on check, fd: %d, buffer: %llx, size: %d", sock, (unsigned long long)buffer, size);
        return -1;
    }

    struct timeval ctv, tv, *ptv;
    ptv = NULL;
    if (timeout > 0)
    {
        tv.tv_sec = timeout / 1000;
        tv.tv_usec = (timeout % 1000) * 1000;
        ctv = tv;
        ptv = &tv;
    }

    for (int wt = 0, left = size, r = 0; left > 0; left -= r, wt += r, ctv = tv)
    {
        fd_set wset;
        FD_ZERO(&wset);
        FD_SET(sock, &wset);
        r = select(sock + 1, NULL, &wset, NULL, ptv);
        if (r == 0)
        {
            return -2;
        }

        r = write(sock, buffer + wt, left);
        if (FD_ISSET(sock, &wset))
        {
            if (r < 0)
            {
                if (errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR)
                {
                    return -1;
                }
                r = 0;
            }
        }
        else
        {
            return -1;
        }
    }

    return 0;
}

static size_t on_write_data(void* buffer, size_t size, size_t nmemb, void* lpVoid)  
{  
    /*
    std::vector<char>* wbuf = dynamic_cast<std::vector<char>*>((std::vector<char> *)lpVoid);  
    if( NULL == wbuf || NULL == buffer )  
    {  
        return -1;  
    }  

    char* pData = (char*)buffer;  
    wbuf->insert(wbuf->end(), pData, pData + size * nmemb);


    msg_t *param = (msg_t *)lpVoid;  
    if (param->server->get_model()->chunk_write(param->task->sock, (char *)buffer, size * nmemb, param->server) == 0)
        return nmemb;  
    return -1;
    */

    msg_t *param = (msg_t *)lpVoid;  
    if (write_with_timeout(param->task->sock, (char *)buffer, size * nmemb, 500) == 0)
        return nmemb;
    return -1;
}  


int send_req(msg_t *msg)
{
    CHttpServer *server = msg->server;
    http_task_t *task = msg->task;
    
    CURLcode res;
    CURL* curl = curl_easy_init();
    if(NULL == curl)
    {
        LOG_WARN("failed to init curl");
        return -1;
    }

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
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, on_write_data);
    //curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&wbuf);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)msg);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
    curl_easy_setopt(curl, CURLOPT_HEADER, 1);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5);
    //curl_easy_setopt(curl, CURLOPT_TIMEOUT, 60);
    curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_0);

    LOG_STAT("send req: %s", task->req.m_header.get_uri());
    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    task->res_done = 1;
    if (res == CURLE_OK)
    {
    }
    else
    {
    }
    server->do_close(task->sock);
    LOG_STAT("req done: %s", task->req.m_header.get_uri());

    return 0;
}

CHttpClient::CHttpClient()
{
}

CHttpClient::CHttpClient(int thread_num, int queue_size):
    CPipeLine(thread_num, queue_size)
{
}

CHttpClient::~CHttpClient()
{
}

int CHttpClient::process()
{
    for (;;)
    {
        msg_t *msg = (msg_t *)m_msg_queue.dequeue();
        if (!msg)
            continue;

        LOG_STAT("dequeue: %s", msg->task->req.m_header.get_uri());
        send_req(msg);
    }
    return 0;
}
