#ifndef __HTTP_RESPONSE_H__
#define __HTTP_RESPONSE_H__

#include "HttpHeader.h"

class CHttpResponse
{
public:
    CHttpResponse();
    ~CHttpResponse();
private:
    CHttpHeader m_header;
};

#endif //__HTTP_RESPONSE_H__
