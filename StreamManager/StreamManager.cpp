#include "StreamManager.h"

stream_manager::stream_manager(int capacity)
    :m_capacity(capacity)
{
    m_stream_data_buffer = (unsigned char*)malloc(m_capacity);
    m_data_length = 0;
}

stream_manager::~stream_manager()
{
    if (NULL != m_stream_data_buffer)
    {
        free(m_stream_data_buffer);
    }
}

void stream_manager::set_capacity_size(int capacity_size)
{
    m_capacity = capacity_size;
    if (m_stream_data_buffer)
    {
        delete m_stream_data_buffer;
    }
    m_stream_data_buffer = (unsigned char*)malloc(m_capacity);
}

int stream_manager::pull_data(void* op, unsigned char* buffer, int length)
{
    //LOG("this = %d, m_data_length = %d\n", this, m_data_length);
    if (NULL == buffer || NULL == m_stream_data_buffer)
    {
        return 0;
    }

    if (length > m_data_length)
    {
        //LOG("stream manager don't have enough data.\n");
        return 0;
    }

    m_lock.lock();
    //LOG("stream_manager pull_data func lock.*********************************\n");

    int i = 0;
    for (; i < length && 0 < m_data_length; i++)
    {
        buffer[i] = m_stream_data_buffer[m_stream_start_point++];

        m_data_length--;
        m_stream_start_point %= m_capacity;
    }

    m_lock.unlock();
    //LOG("stream_manager pull_data func unlock.*********************************\n");

    return i;
}


int stream_manager::push_data(unsigned char* data, int length)
{
    //LOG("this = %d, m_data_length = %d\n", this, m_data_length);
    if (NULL == data || NULL == m_stream_data_buffer)
    {
        return 0;
    }

    if (length > m_capacity - m_data_length)
    {
        //LOG("stream manager don't have enough space.\n");
        return 0;
    }

    m_lock.lock();
    //LOG("stream_manager push_data func lock.#####################################\n");

    int i = 0;
    for (; i < length && m_data_length < m_capacity; i++)
    {
        m_stream_data_buffer[m_stream_end_point++] = data[i];

        m_data_length ++;
        m_stream_end_point %= m_capacity;
    }

    m_lock.unlock();
    //LOG("stream_manager push_data func unlock.#####################################\n");

    return i;
}

int stream_manager::get_data_length()
{
    return m_data_length;
}