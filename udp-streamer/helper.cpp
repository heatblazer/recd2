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

        QList<utils::frame_data_t> dbl;

        do {

            h->m_lock.lock();
            while (!h->m_buffer.isEmpty()) {
                dbl.append(h->m_buffer.takeFirst());
            }
            h->m_lock.unlock();
            // do the peek check of N frames

            if (dbl.count() < h->m_packSize) {
                continue;
            } else {
                // how much frames we`ve buffered so far
                int max_samples = s->m_channels * s->m_smplPerChan * dbl.count();
                int16_t* smpls = new int16_t[max_samples];
                int index = 0;

                for(int i=0; i < dbl.count(); ++i) {
                    utils::frame_data_t frame = dbl.at(i);
                    for(int j=0; j < s->m_channels; ++j) {
                        for(int h=0; h < s->m_smplPerChan; ++h) {
                            smpls[index++] = frame.data[j * s->m_smplPerChan + h];
                        }
                    }
                }

                for(int j=0; j < s->m_smplPerChan; ++j) {
                    for(int k=0; k < s->m_channels; ++k) {
                        h->peek(smpls[j * s->m_channels + k]);
                        printf("Chan: (%d) <====> peek: (%d)\n", k,
                               h->m_peek);
                   }
                }
                h->m_peek = 0;
                // don`t forget!
                delete [] smpls;
                smpls = nullptr;
                dbl.clear();
            }

            h->suspend(100);

        } while (h->m_isRunning);

        return (int*)0;
    }

    Helper::Helper(size_t packSize)
        : m_packSize(packSize),
          m_peek(0),
          m_isRunning(false)
    {
        m_lock.init();
    }


    Helper::~Helper()
    {
        m_lock.deinit();
    }

    int16_t Helper::peek(int16_t val)
    {
        int16_t v = abs(val);
        if (m_peek < v) {
            m_peek = v;
        }
        return m_peek;
    }

    } // udp
} // plugin
