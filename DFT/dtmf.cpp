#include "dtmf.h"

#include <iostream>

#define FRAME_SIZE 160

static char s_DialButtons[16] ={'1' , '2', '3',
                                'A', '4', '5',
                                '6', 'B', '7',
                                '8', '9', 'C',
                                '*', '0', '#', 'D'
                               };


static inline bool beginDTMF(short int peek)
{
    bool res = false;
    switch (peek) {
    default:
        res = false;
        break;
    }

    return res;
}

namespace plugin
{

    namespace dtmf
    {

    Dtmf* Dtmf::s_inst = nullptr;

    Dtmf &Dtmf::Instance()
    {
        if (s_inst == nullptr) {
            s_inst = new Dtmf;
        }
        return *s_inst;
    }

    void Dtmf::init()
    {
        Dtmf* d = &Instance();
        d->setObjectName("dtmf-detector");
        d->m_isRunning = true;
        d->start();
    }

    void Dtmf::deinit()
    {
        Dtmf* d = &Instance();
        utils::IPC::Instance().sendMessage("deinit dtmf detector...\n");
        d->wait(1000);
    }

    ///@Unused
    void Dtmf::copy(const void *src, void *dst, int len)
    {
        (void) src; (void) dst; (void) len;
    }

    int Dtmf::put_data(void *data)
    {
        Dtmf* d = &Instance();
        QList<utils::sample_data_t>* ls = (QList<utils::sample_data_t>*)data;
        d->m_lock.lock();
        if (!ls->isEmpty()) {
            for(int i=0; i < ls->count(); ++i) {
                d->m_sampleBuffer.data.append(ls->at(i));
            }
        }
        d->m_lock.unlock();

        // pass to next, dont slow down
        if (d->iface.nextPlugin != nullptr) {
            d->iface.nextPlugin->put_data(data);
        }
        // copy the data to local buffer
    }

    ///@Unused
    int Dtmf::put_ndata(void *data, int len)
    {
        (void) data; (void) len;
    }

    void *Dtmf::get_data()
    {
        return nullptr;
    }

    void Dtmf::setName(const char *name)
    {
        Dtmf* d = &Instance();
        strncpy(d->iface.name, name, sizeof(d->iface.name));
    }

    const char *Dtmf::getName()
    {
        return Instance().getName();
    }

    int Dtmf::p_main(int argc, char **argv)
    {
        // init something from config file here...
        (void) argc; (void) argv;
    }

    interface_t *Dtmf::getSelf()
    {
        Dtmf* r = &Instance();
        return &r->iface;
    }

    ///Overriden run of QThread
    /// \brief Dtmf::run
    ///
    void Dtmf::run()
    {
        QList<utils::sample_data_t> dbl;
        int buffSize = 0;
        while(m_isRunning) {

            usleep(1);

            // roweork the logic here !!!...
            // wait to pack a whole defined frame
           buffSize = m_sampleBuffer.data.count();
           if (buffSize < FRAME_SIZE) {
               continue;
           }
           m_lock.lock();
           utils::sample_data_t s;

           for(int i=0; i < buffSize ; ++i){
               dbl.append(m_sampleBuffer.data.at(i));
           }
           m_sampleBuffer.data.clear();
           m_lock.unlock();
           for(int it=0; it < dbl.count(); ++it) {
               short int dtmf[FRAME_SIZE];// = new short int[size];

               for(int i=0; i < FRAME_SIZE; ) {
                   if (dbl.isEmpty()) {
                       break;
                   }
                   utils::sample_data_t smpl = dbl.at(it);
                   for(int j=0; j < smpl.size; ++j) {
                       dtmf[i++] = smpl.samples[j];
                   }
               }

               m_dtmfDetector.dtmfDetecting((INT16*)dtmf);

               if (m_dtmfDetector.getIndexDialButtons() < 1) {
                  // printf("Error in detecting number of buttons\n");
                   continue;
               }
               for(int ii = 0; ii < m_dtmfDetector.getIndexDialButtons(); ++ii) {
                   if(m_dtmfDetector.getDialButtonsArray()[ii] != s_DialButtons[ii])
                   {
                   //    printf("Error of a detecting button \n");
                       continue;
                   } else {
                       printf("We got: [%c]\n", s_DialButtons[ii]);
                   }
               }
                // clear the double buffer
           } // end if performing dtmf detection
        } // busy loop
    }

    Dtmf::Dtmf(QThread *parent)
        : QThread(parent),
          m_isRunning(false),
          m_dtmfDetector(FRAME_SIZE, 102) // experimental!!!
    {
        m_sampleBuffer.is_busy = false;
    }

    Dtmf::~Dtmf()
    {
        if (!m_sampleBuffer.data.isEmpty()) {
            m_sampleBuffer.data.clear();
        }
    }

    } // dtmf
} // plugin


////////////////////////////////////////////////////////////////////////////////
const interface_t *get_interface()
{
    interface_t* piface = plugin::dtmf::Dtmf::Instance().getSelf();

    piface->init = &plugin::dtmf::Dtmf::init;
    piface->deinit = &plugin::dtmf::Dtmf::deinit;
    piface->copy = &plugin::dtmf::Dtmf::copy;
    piface->put_data = &plugin::dtmf::Dtmf::put_data;
    piface->put_ndata = &plugin::dtmf::Dtmf::put_ndata;
    piface->get_data = &plugin::dtmf::Dtmf::get_data;
    piface->main_proxy = &plugin::dtmf::Dtmf::p_main;
    piface->getSelf   = &plugin::dtmf::Dtmf::getSelf;
    piface->setName = &plugin::dtmf::Dtmf::setName;
    piface->getName = &plugin::dtmf::Dtmf::getName;
    piface->nextPlugin = nullptr;

    return plugin::dtmf::Dtmf::Instance().getSelf();
}
