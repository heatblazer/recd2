#include "ring-buffer.h"

#include <string.h>

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
    rHead = &m_buffer[0];
    wHead = &m_buffer[0];
}

int RingBuffer::readAll(frame_data_t **ret)
{
    int diff = rwDiff();
    if (diff == 0) {
        (*ret) = nullptr;
        return 0;
    }
    frame_data_t* d = new frame_data_t[diff];
    for(int i=0; i < diff; ++i){
        d[i] = read();
    }
    (*ret) = d;
    return diff;
}

void RingBuffer::write(frame_data_t &t)
{
    *wHead = t;
    advanceWriteHead();
}

frame_data_t RingBuffer::read()
{
    frame_data_t ret = {0, {0}, {{0}}};
    ret = *rHead;
    advanceReadHead();
    return ret;
}

/// clear ring buffer data
/// \brief RingBuffer::clear
///
void RingBuffer::clear()
{
    memset(&m_buffer, 0, sizeof(m_buffer));
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

const frame_data_t *RingBuffer::begin() const
{
    return &m_buffer[0];
}

const frame_data_t *RingBuffer::end() const
{
    return &m_buffer[RingBuffer::SIZE-1];
}

} // utils
