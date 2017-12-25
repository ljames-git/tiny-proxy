#include <vector>
#include <string.h>

#include "curl/curl.h"

#include "msg.h"
#include "common.h"
#include "HttpClient.h"

static size_t OnWriteData(void* buffer, size_t size, size_t nmemb, void* lpVoid)  
{  
    /*
    std::vector<char>* wbuf = dynamic_cast<std::vector<char>*>((std::vector<char> *)lpVoid);  
    if( NULL == wbuf || NULL == buffer )  
    {  
        return -1;  
    }  

    char* pData = (char*)buffer;  
    wbuf->insert(wbuf->end(), pData, pData + size * nmemb);
    */
    msg_t *param = (msg_t *)lpVoid;  
    param->server->get_model()->chunk_write(param->task->sock, (char *)buffer, size * nmemb, param->server);
    return nmemb;  
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
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnWriteData);
    //curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&wbuf);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)msg);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
    curl_easy_setopt(curl, CURLOPT_HEADER, 1);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5);
    //curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5);
    curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_0);
    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res == CURLE_OK)
    {
        char x[] = "\r\n";
        server->get_model()->write(task->sock, x, strlen(x), server);
        //server->get_model()->write(task->sock, wbuf.data(), wbuf.size(), server);
    }
    else
    {
        server->send_404(task);
        LOG_WARN("404 from %s, ret: %d", url.c_str(), res);

        /*
        char **p = header.get_keys(&size);
        for (char **q = p; q - p < size; q++)
        {
            char buf[1024];
            snprintf(buf, sizeof(buf), "%s:%s", *q, header.get_value(*q));
            LOG_WARN(buf);
        }
        delete []p;
        */
    }

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
        send_req(msg);
    }
    return 0;
}
