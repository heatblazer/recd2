#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>
namespace utils {
// opacity
struct udp_data_t;
struct tcp_data_t;

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

} // utils
#endif // TYPES_H
