#ifndef __RW_OBJECT_H__
#define __RW_OBJECT_H__

class CRwObject
{
public:
    ~CRwObject();

    static CRwObject *create_object(int size);

    char *m_buffer;
    int m_total_size;
    int m_left_size;
    int m_done_size;

private:
    CRwObject();
    CRwObject(int size);
};

#endif //__RW_OBJECT_H__
