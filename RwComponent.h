#ifndef __RW_COMPONENT_H__
#define __RW_COMPONENT_H__

class IRwComponent
{
    protected:
        virtual int do_read(int sock, char *buf, int size) = 0;
        virtual int do_write(int sock, char *buf, int size) = 0;
};

#endif //__RW_COMPONENT_H__
