#include "HttpHeader.h"

CHttpHeader::CHttpHeader()
{
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
    return 0;
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
