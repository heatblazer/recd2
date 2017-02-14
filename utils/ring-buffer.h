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
    int readAll(udp_data_t** ret);
    void write(udp_data_t& t);
    udp_data_t &read();

private:

    void advanceWriteHead();
    void advanceReadHead();
    int  rwDiff();

    enum MaxSize
    {
        SIZE = 2048
    };
    udp_data_t* rHead;
    udp_data_t* wHead;

    const udp_data_t* begin() const;
    const udp_data_t* end() const;

    udp_data_t m_buffer[RingBuffer::MaxSize::SIZE];


};

} // utils

#endif // RINGBUFFER_H
