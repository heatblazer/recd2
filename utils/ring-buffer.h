#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#include "types.h"

namespace utils {

template <typename T, size_t SIZE=512> class RingBuffer
{

public:
    RingBuffer()
        : rHead(nullptr),
          wHead(nullptr)
    {
    }

    ~RingBuffer()
    {

    }

    void init()
    {
        rHead = &m_buffer[0];
        wHead = &m_buffer[0];
    }

    int readAll(T** ret)
    {
        int diff = rwDiff();
        if (diff == 0) {
            (*ret) = nullptr;
            return 0;
        }
        T* d = new T[diff];
        for(int i=0; i < diff; ++i){
            d[i] = *read();
        }
        (*ret) = d;
        return diff;
    }

    void write(T& t)
    {
        *wHead = t;
        advanceWriteHead();
    }

    T *read()
    {
        T *ret = nullptr;
        if (rHead == wHead) {
            return nullptr;
        } else {
            ret = rHead;
            advanceReadHead();
        }
        return ret;
    }

    void clear()
    {
        memset(&m_buffer, 0, sizeof(m_buffer));
    }

private:

    void advanceWriteHead()
    {
        if (wHead == end() ) {
            wHead = &m_buffer[0];
        } else {
            wHead++;
        }
    }

    void advanceReadHead()
    {
        if (rHead == end()) {
            rHead = &m_buffer[0];
        } else {
            rHead++;
        }
    }

    int  rwDiff()
    {
        return wHead - rHead;
    }

    T* rHead;
    T* wHead;

    const T* begin() const
    {
        return &m_buffer[SIZE-1];

    }

    const T* end() const
    {
        return &m_buffer[SIZE-1];
    }

    T m_buffer[SIZE];
};

} // utils

#endif // RINGBUFFER_H
