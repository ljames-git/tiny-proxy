#ifndef __RW_COMPONENT_H__
#define __RW_COMPONENT_H__

class IRwComponent
{
public:
    virtual bool is_acceptable(int sock) = 0;
    virtual int on_accept(int sock) = 0;
    virtual int on_data(int sock, char *buf, int size) = 0;
    virtual int on_close(int sock) = 0;
    virtual int on_error(int sock) = 0;
    virtual int on_write_done(int sock) = 0;
};

#endif //__RW_COMPONENT_H__
