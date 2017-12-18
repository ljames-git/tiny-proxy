#ifndef __HTTP_SERVER_H__
#define __HTTP_SERVER_H__

#include <map>
#include "TcpServer.h"
#include "HttpRequest.h"
#include "HttpResponse.h"

struct http_task_t
{
    int sock;                           // socket
    int body_offset;                    // body position in header buf
    int header_size;                    // the size of header
    int header_buf_size;                // valid bytes in header_buf
    int body_size;                      // current body size
    int req_done;                       // body received done
    char *body;
    CHttpRequest req;
    CHttpResponse res;
    char header_buf[HTTP_HEADER_LEN];

    http_task_t(int s):
        sock(s),
        body_offset(0),
        header_size(0),
        header_buf_size(0),
        body_size(0),
        req_done(0),
        body(NULL)
    {
        header_buf[0] = 0;
    }

    ~http_task_t()
    {
        if (body)
            delete []body;
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
    virtual int send_404(http_task_t *task);

protected:
    virtual int do_close(int sock);

private:
    int parse_req_header(http_task_t *task);
    int task_done(http_task_t *task);

    task_map_t m_task_map;
};

#endif //__HTTP_SERVER_H__
