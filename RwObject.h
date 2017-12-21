#ifndef __RW_OBJECT_H__
#define __RW_OBJECT_H__

class CRwObject
{
public:
    ~CRwObject();

    static CRwObject *create_object(int size, bool is_last = false);

    char *m_buffer;
    bool m_is_last;
    int m_total_size;
    int m_left_size;
    int m_done_size;

private:
    CRwObject();
    CRwObject(int size, bool is_last);
};

#endif //__RW_OBJECT_H__
