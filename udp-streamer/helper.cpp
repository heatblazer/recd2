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

        QList<utils::frame_data_t> dbl;

        do {
            h->m_lock.lock();
            while (!h->m_buffer.isEmpty()) {
                dbl.append(h->m_buffer.takeFirst());
            }
            h->m_lock.unlock();
            // do the peek check of N frames
#if 1
#else
            if (dbl.count() < h->m_packSize) {
                continue;
            } else {
                int size = dbl.count();
                int peek_samples = size * s->m_smplPerChan;
                // how much frames we`ve buffered so far
                int16_t** smpls = new int16_t *[s->m_channels];
                for(int i=0; i < s->m_channels; ++i) {
                    smpls[i] = new int16_t[peek_samples];
                }

                int p = 0;
                for(int i=0; i < size; ++i) {
                    utils::frame_data_t frame = dbl.takeAt(i);
                    for(uint32_t j=0; j < s->m_channels; ++j) {
                        for(uint32_t k=0; k < s->m_smplPerChan; ++k) {
                            smpls[j][p++ % peek_samples] = frame.data[k * s->m_channels + j];
                        }
                    }
                }

                for(uint32_t i=0; i < s->m_channels; ++i) {
                    int16_t p = h->peek(smpls[i], size);
                    printf("Channel: [%2d] ------ Peek: [%2d]\n",
                           i, p);
                    h->m_peek = 0;
                }

                // don`t forget!
             //   for(int i=0; i < s->m_channels; ++i) {
             //       delete [] &smpls[i];
             //   }
             //   delete [] smpls;
            }
            dbl.clear();
#endif
            h->suspend(100);

        } while (h->m_isRunning);

        return (int*)0;
    }

    Helper::Helper(size_t packSize)
        : m_packSize(packSize),
          m_isRunning(false),
          m_peek(0)
    {
        m_lock.init();
    }


    Helper::~Helper()
    {
        m_lock.deinit();
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
