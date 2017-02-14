#include "dtmf.h"


#define FRAME_SIZE 16

// local data for pressed buttons
static char s_DialButtons[16] =
    {'1' , '2', '3',
     'A', '4', '5',
    '6', 'B', '7',
     '8', '9', 'C',
    '*', '0', '#', 'D'};

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
        if (d->iface.nextPlugin != nullptr) {
            d->iface.nextPlugin->put_data(data);
        }

        QList<utils::sample_data_t>* ls = (QList<utils::sample_data_t>*)data;
        d->m_lock.lock();
        for(int i=0; i < ls->count(); ++i) {
            d->m_sampleBuffer.append(ls->at(i));
        }
        d->m_lock.unlock();
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
        do {
           m_lock.lock();
           while (!m_sampleBuffer.isEmpty()) {
               dbl.append(m_sampleBuffer.takeFirst());
           }
           m_lock.unlock();
           while(!dbl.isEmpty()) {
               utils::sample_data_t s = dbl.takeFirst();
               m_dtmfDetector.dtmfDetecting(s.samples);
               if (m_dtmfDetector.getIndexDialButtons() != 16) {
                   printf("Error in detecting number of buttons\n");
               }

               for(int i=0; i < 16; ++i) {
                   if (m_dtmfDetector.getDialButtonsArray()[i] != s_DialButtons[i]) {
                        printf("Error detecting button! \n");
                   } else {
                       printf("We got: %c\n", s_DialButtons[i]);
                   }
               }

           }
           // algo is not so important so can sleep a bit
           sleep(50);
        } while (m_isRunning);
    }

    Dtmf::Dtmf(QThread *parent)
        : QThread(parent),
          m_isRunning(false),
          m_dtmfDetector(FRAME_SIZE) // experimental!!!
    {

    }

    Dtmf::~Dtmf()
    {

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
