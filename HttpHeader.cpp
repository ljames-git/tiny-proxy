#include <string.h>
#include "HttpHeader.h"

CHttpHeader::CHttpHeader():
    m_method(HTTP_METHOD_NONE),
    m_content_length(0)
{
    m_uri[0] = 0;
}

CHttpHeader::~CHttpHeader()
{
}

const char * CHttpHeader::get_value(const char *key)
{
    if (m_map.find(key) == m_map.end())
        return NULL;

    return m_map[key].c_str();
}

int CHttpHeader::set_value(const char *key, const char *value)
{
    int ret = m_map.find(key) != m_map.end();
    m_map[key] = value;
    return ret;
}

char ** CHttpHeader::get_keys(int *count)
{
    if (count == NULL)
        return NULL;

    *count = m_map.size();
    char **keys = new char *[*count];
    if (keys == NULL)
        return NULL;

    int i = 0;
    for (std::map<std::string, std::string>::iterator it = m_map.begin(); it != m_map.end(); it++)
    {
        keys[i++] = const_cast<char *>((it->first).c_str());
    }

    return keys;
}

int CHttpHeader::get_method()
{
    return m_method;
}

int CHttpHeader::set_method(int method)
{
    m_method = method;
    return 0;
}

const char *CHttpHeader::get_uri()
{
    return m_uri;
}

int CHttpHeader::set_uri(const char* uri)
{
    int len = strlen(uri);
    if (len >= HTTP_URI_LEN)
        return -1;

    strcpy(m_uri, uri);
    return 0;
}

int CHttpHeader::get_content_length()
{
    return m_content_length;
}

int CHttpHeader::set_content_length(int length)
{
    if (length < 0)
        return -1;

    m_content_length = length;
    return 0;
}
