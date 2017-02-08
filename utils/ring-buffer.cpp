#include "ring-buffer.h"

namespace utils {

RingBuffer::RingBuffer()
    : rHead(nullptr),
      wHead(nullptr)
{
}

RingBuffer::~RingBuffer()
{
}

void RingBuffer::init()
{
    rHead = m_buffer;
    wHead = m_buffer;
}

void RingBuffer::write(udp_data_t &t)
{
    if ((rwDiff() >= 0)) {
        wHead = &t;
        advanceWriteHead();
    }
}

udp_data_t &RingBuffer::read()
{
    udp_data_t ret = {0, {0}, {{0}}};
    if ((rwDiff() >= 0)) {
        ret = *rHead;
        advanceReadHead();
    }
    return ret;
}

void RingBuffer::advanceWriteHead()
{
    if (wHead == end() ) {
        wHead = &m_buffer[0];
    } else {
        wHead++;
    }
}

void RingBuffer::advanceReadHead()
{
    if (rHead == end()) {
        rHead = &m_buffer[0];
    } else {
        rHead++;
    }
}

/// 0 if RW on same place
/// 1 if W is ahead
/// -1 R is ahead - should never happen
/// \brief RingBuffer::rwDiff
/// \return
///
int RingBuffer::rwDiff()
{
    return wHead - rHead;
}

const udp_data_t *RingBuffer::begin() const
{
    return &m_buffer[0];
}

const udp_data_t *RingBuffer::end() const
{
    return &m_buffer[RingBuffer::SIZE-1];
}

} // utils
