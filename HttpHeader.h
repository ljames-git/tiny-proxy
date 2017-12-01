#ifndef __HTTP_HEADER_H__
#define __HTTP_HEADER_H__

#include <map>
#include <string>

#define HTTP_URI_LEN 8192
#define HTTP_COOKIE_LEN 5000
#define HTTP_HEADER_LEN 65536

#define HTTP_METHOD_NONE 0
#define HTTP_METHOD_OPTIONS 1
#define HTTP_METHOD_GET 2
#define HTTP_METHOD_POST 3
#define HTTP_METHOD_CONNECT 4

class CHttpHeader
{
public:
    CHttpHeader();
    virtual ~CHttpHeader();

    // key and values
    const char *get_value(const char *key);
    int set_value(const char *key, const char *value);
    char ** get_keys(int *count);

    // method
    int get_method();
    int set_method(int method);

    // uri
    const char *get_uri();
    int set_uri(const char* uri);

    // content-length
    int get_content_length();
    int set_content_length(int length);

private:
    int m_method;
    int m_content_length;
    char m_uri[HTTP_URI_LEN];
    std::map<std::string, std::string> m_map;
};

#endif //__HTTP_HEADER_H__
