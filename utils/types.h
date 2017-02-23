#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>
#include <stdio.h>

namespace utils {

// I am having some troubles playing with bits...
// now I am going to use a nice template structure
// that displays all of them
// this will inspect individual BITS of a data type
// int for example usedful for sample data and stuff alike.
// not sure if this compiles under C++11 because union templates are
// non 98 standart compilant.
template <typename T> union bytes_t
{
    T var;
    struct b {
        uint8_t a: 1;
        uint8_t b: 1;
        uint8_t c: 1;
        uint8_t d: 1;
        uint8_t e: 1;
        uint8_t f: 1;
        uint8_t g: 1;
        uint8_t h: 1;
    } bits[sizeof(T)];
};


/// TODO: for future it will be setup dynamically
/// from the config file
/// postimplemented struct for udp header
/// \brief The frame_data_t struct
///
struct frame_data_t
{
    uint32_t counter;
    uint8_t null_bytes[64];
    int16_t data[32 * 16]; // the new concpet - max is 1024 bytes
};

// required for flippingp_server
struct frame_data_t2
{
    uint32_t    counter;
    uint8_t     null_bytes[32];
    int16_t     data[16][32];
};

struct sample_data_t
{
    short* samples;
    uint32_t size;
};
struct wav_hdr_t
{
    char riff_tag[4];
    int  riff_len;
    char wav_tag[4];
    char fmt_tag[4];
    int fmt_len;
    short audio_format;
    short num_channels;
    int sample_rate;
    int byte_rate;
    short block_align;
    short bits_per_sample;
    char data_tag[4];
    int data_len;
};


/// maybe inherit QPair
/// simple pair template
/// to access key by value
template <typename T1, typename T2> struct MPair
{
    MPair() {}

    MPair(const T1& t1, const T2& t2) :
        m_type1(t1), m_type2(t2)
    {}

    MPair(const MPair& ref)
    {
        m_type1 = ref.m_type1;
        m_type2 = ref.m_type2;
    }

    bool operator ==(const MPair& ref)
    {
        return (ref.m_type1 == ref.m_type1) &&
                (ref.m_type2 == ref.m_type2);
    }

    const MPair& operator=(const MPair& ref)
    {
        return ref;
    }

    const T2& operator[] (const T1& t1)
    {
        if(t1 == m_type1) {
            return m_type2;
        } else {
            static T2 none;
            return none;
        }
    }

    T1 m_type1;
    T2 m_type2;
};

// IPC stuff
struct msg_t
{
    long val; // this is needed
    char buff[512]; // no more than 512 bytes messages plase
};

} // utils
#endif // TYPES_H
