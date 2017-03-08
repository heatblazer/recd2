#include "helper.h"

#include <math.h>

#include "server.h"

namespace plugin {

    namespace udp
    {

    void *Helper::worker(void *pArgs)
    {
        Helper* h = (Helper*) pArgs;
        Server* s = &Server::Instance();
        (void) s;
        h->suspend(100);
        QList<utils::frame_data_t> dbl;


        int16_t peeks[64] = {0};

        printf("(%5d)(%5d)(%5d)(%5d)(%5d)(%5d)(%5d)(%5d)(%5d)(%5d)"
               "(%5d)(%5d)(%5d)(%5d)(%5d)(%5d)(%5d)(%5d)(%5d)(%5d)"
               "(%5d)(%5d)(%5d)(%5d)(%5d)(%5d)(%5d)(%5d)(%5d)(%5d)"
               "(%5d)(%5d)(%5d)(%5d)(%5d)(%5d)(%5d)(%5d)(%5d)(%5d)"
               "(%5d)(%5d)(%5d)(%5d)(%5d)(%5d)(%5d)(%5d)(%5d)(%5d)"
               "(%5d)(%5d)(%5d)(%5d)(%5d)(%5d)(%5d)(%5d)(%5d)(%5d)"
               "(%5d)(%5d)(%5d)(%5d)\n", 1, 2, 3, 4, 5, 6, 7, 8, 9,
               10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22,
               23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38,
               38, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53,
               54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64);

        do {

            h->m_lock.lock();
            while (!h->m_buffer.isEmpty()) {
                dbl.append(h->m_buffer.takeFirst());
            }
            h->m_lock.unlock();
            // do the peek check of N frames
            if (dbl.count() < Server::s_peekOptions.peekSize) {
                continue;
            } else {
                int size = dbl.count();
                int16_t* smpls = new int16_t[s->m_smplPerChan * size];
                int sindex = 0, ssize = s->m_smplPerChan * size;

                for(int i=0; i < size; ++i) {
                    utils::frame_data_t fd = dbl.at(i);
                    for(int j=0; j < s->m_channels; ++j) {
                        for(int k=0; k < s->m_smplPerChan; ++k) {
                            smpls[sindex++ % ssize] = fd.data[k * s->m_channels + j];
                        }
                        int16_t peek = h->peek(smpls, ssize);
                        peeks[j] = peek;
                        h->m_peek = 0;
                        sindex = 0;
                    }
                }

                printf("(%5d)(%5d)(%5d)(%5d)(%5d)(%5d)(%5d)(%5d)(%5d)(%5d)"
                       "(%5d)(%5d)(%5d)(%5d)(%5d)(%5d)(%5d)(%5d)(%5d)(%5d)"
                       "(%5d)(%5d)(%5d)(%5d)(%5d)(%5d)(%5d)(%5d)(%5d)(%5d)"
                       "(%5d)(%5d)(%5d)(%5d)(%5d)(%5d)(%5d)(%5d)(%5d)(%5d)"
                       "(%5d)(%5d)(%5d)(%5d)(%5d)(%5d)(%5d)(%5d)(%5d)(%5d)"
                       "(%5d)(%5d)(%5d)(%5d)(%5d)(%5d)(%5d)(%5d)(%5d)(%5d)"
                       "(%5d)(%5d)(%5d)(%5d)",
                       peeks[0], peeks[1], peeks[2], peeks[3], peeks[4],
                        peeks[5], peeks[6], peeks[7], peeks[8], peeks[9],
                        peeks[10], peeks[11], peeks[12], peeks[13], peeks[14],
                        peeks[15], peeks[16], peeks[17], peeks[18], peeks[19],
                        peeks[20], peeks[21], peeks[22], peeks[23], peeks[24],
                        peeks[25], peeks[26], peeks[27], peeks[28], peeks[29],
                        peeks[30], peeks[31], peeks[32], peeks[33], peeks[34],
                        peeks[35], peeks[36], peeks[37], peeks[38], peeks[39],
                        peeks[40],peeks[41], peeks[42], peeks[43], peeks[44],
                        peeks[45], peeks[46], peeks[47], peeks[48], peeks[49],
                        peeks[50], peeks[51], peeks[52], peeks[53], peeks[54],
                        peeks[55], peeks[56], peeks[57], peeks[58], peeks[59],
                        peeks[60], peeks[61], peeks[62], peeks[63]);


                if (smpls != nullptr) {
                    delete [] smpls;
                }
                dbl.clear();
            }
            h->suspend(100);

        } while (h->m_isRunning);

        return (int*)0;
    }

    Helper::Helper()
        : m_packSize(0),
          m_isRunning(false),
          m_peek(0)
    {
        m_lock.init();
    }


    Helper::~Helper()
    {
        m_lock.deinit();
    }

    void Helper::setPacketSize(int s)
    {
        m_packSize = s;
    }

    int16_t Helper::peek(int16_t *samples, int size)
    {
        if (samples == nullptr || size == 0) {
            return -1;
        }
        for(int i=0; i < size; ++i) {
            if (m_peek < abs(samples[i])) {
                m_peek = abs(samples[i]);
            }
        }
        return m_peek;
    }

    } // udp
} // plugin
