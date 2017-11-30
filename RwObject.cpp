#include <string.h>
#include "RwObject.h"

CRwObject::CRwObject() :
    m_buffer(NULL),
    m_total_size(0),
    m_left_size(0),
    m_done_size(0)
{
}

CRwObject::CRwObject(int size) :
    m_total_size(size),
    m_left_size(size),
    m_done_size(0)
{
    m_buffer = new char[size];
}

CRwObject::~CRwObject()
{
    if (m_buffer)
        delete []m_buffer;
}

CRwObject *CRwObject::create_object(int size)
{
    if (size <= 0)
        return NULL;

    CRwObject *obj = new CRwObject(size);
    if (obj == NULL || obj->m_buffer == NULL)
        return NULL;

    return obj;
}
