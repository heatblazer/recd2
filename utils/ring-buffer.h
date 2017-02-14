#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#include "types.h"

namespace utils {

class RingBuffer
{
public:
    RingBuffer();
    ~RingBuffer();
    void init();
    int readAll(frame_data_t** ret);
    void write(frame_data_t& t);
    frame_data_t *read();
    void clear();

private:

    void advanceWriteHead();
    void advanceReadHead();
    int  rwDiff();

    enum MaxSize
    {
        SIZE = 256
    };
    frame_data_t* rHead;
    frame_data_t* wHead;

    const frame_data_t* begin() const;
    const frame_data_t* end() const;

    frame_data_t m_buffer[RingBuffer::MaxSize::SIZE];


};

} // utils

#endif // RINGBUFFER_H
