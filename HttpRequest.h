#ifndef __HTTP_REQUEST_H__
#define __HTTP_REQUEST_H__

#include "HttpHeader.h"

class CHttpRequest
{
public:
    CHttpRequest();
    ~CHttpRequest();

    CHttpHeader m_header;
};

#endif //__HTTP_REQUEST_H__
