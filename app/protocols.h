#ifndef PROTOCOLS_H
#define PROTOCOLS_H

#include <stdint.h>

namespace recd
{
/// what a rtp header should be
/// \brief The rtp_hdr_t struct
///
struct rtp_hdr_t
{
    union {
        struct {
            unsigned int V :    2; // version (always 2)
            unsigned int P :    1; // padding
            unsigned int X :    1; // extension
            unsigned int CSRC : 4; // count (CC)
            unsigned int M :    1; // mark bit
            unsigned int PT :   7; // payload type
            unsigned int SC :   16; // sequence number
        } f;
        int32_t i;
    } u;
    int32_t time_stamp; // timestampe, init value should be random!
    int32_t SSRC;       // syncrhonization identifier, should be random
    int32_t CSRC[1];    // 0 - 15 items contrib sources
};

}  // recd


#endif // PROTOCOLS_H
