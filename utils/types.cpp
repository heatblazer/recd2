#include "types.h"




uint8_t utils::gen_checksum_lrc(struct frame_data_t *fd)
{
    uint8_t checksum = 0;
    int i;

    uint8_t* p = (uint8_t*)&fd->counter; // go to counter

    for(i=0; i < sizeof(fd->counter); i++) {
        checksum += p[i] & 0xff;
    }

    p = &fd->null_bytes[0];//go to null bytes,
    for(i=0; i < sizeof(fd->null_bytes); i++) {
        checksum += p[i] & 0xff;
    }

    p = (uint8_t*)&fd->data[0]; // go to data
    for(i=0; i < sizeof(fd->data)/sizeof(fd->data[0]); i++) {
        checksum += p[i] & 0xff;
    }

    checksum = ((checksum ^ 0xff)+1) & 0xff; //calc the lrc checksum

    fd->checksum = checksum;

    return checksum;
}
