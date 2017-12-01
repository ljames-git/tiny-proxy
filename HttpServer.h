#ifndef __HTTP_SERVER_H__
#define __HTTP_SERVER_H__

#include <map>
#include "TcpServer.h"
#include "HttpRequest.h"
#include "HttpResponse.h"

struct http_task_t
{
    int sock;
    int body_offset;
    int header_size;
    int header_buf_size;
    CHttpRequest req;
    CHttpResponse res;
    char header_buf[HTTP_HEADER_LEN];

    http_task_t(int s):
        sock(s),
        body_offset(0),
        header_size(0),
        header_buf_size(0)
    {
        header_buf[0] = 0;
    }
};


class CHttpServer: public CTcpServer
{

typedef std::map<int, http_task_t *> task_map_t;

public:
    CHttpServer();
    CHttpServer(int port);
    ~CHttpServer();

public:
    virtual int on_data(int sock, char *buf, int size);

protected:
    virtual int do_close(int sock);

private:
    int parse_req_header(http_task_t *task);
    task_map_t m_task_map;
};

#endif //__HTTP_SERVER_H__
