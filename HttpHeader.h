#ifndef __HTTP_HEADER_H__
#define __HTTP_HEADER_H__

#include <map>
#include <string>

class CHttpHeader
{
public:
    CHttpHeader();
    virtual ~CHttpHeader();

    const char *get_value(const char *key);
    int set_value(const char *key, const char *value);
    char ** get_keys(int *count);

private:
    std::map<std::string, std::string> m_map;
};

#endif //__HTTP_HEADER_H__
