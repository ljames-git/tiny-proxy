#ifndef __RW_COMPONENT_H__
#define __RW_COMPONENT_H__

class IRwComponent
{
    public:
        virtual bool is_acceptable(int sock) = 0;
        virtual int do_accept(int sock) = 0;
        virtual int do_clean(int sock) = 0;
        virtual int do_read(int sock, char *buf, int size) = 0;
        virtual int do_write(int sock, char *buf, int size) = 0;
};

#endif //__RW_COMPONENT_H__
