#include "dtmf.h"

#include <stdio.h>
#include <string.h>


static const char* THIS_FILE = "dtmf.cpp";


static char s_DialButtons[] ={
                                '1' , '2', '3',
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

    int Dtmf::s_freq = 0;
    int Dtmf::s_frameSize = 0;

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
        d->m_dtmfDetector = new DtmfDetector(Dtmf::s_frameSize, Dtmf::s_freq);
        d->setObjectName("dtmf-detector");
        d->m_isRunning = true;
        d->start();
    }

    void Dtmf::deinit()
    {
        Dtmf* d = &Instance();
        utils::IPC::Instance().sendMessage(THIS_FILE, "deinit dtmf detector...\n");
        d->m_isRunning = false;
        d->usleep(100);
        d->wait(1000);
        if (d->m_dtmfDetector != nullptr) {
            delete d->m_dtmfDetector;
            d->m_dtmfDetector = nullptr;
        }
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
        return 0;
    }

    ///@Unused
    int Dtmf::put_ndata(void *data, int len)
    {
        (void) data; (void) len;
        return 0;
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

    /// configure the dtmf here from external xml file which
    /// path is in the 'conf' attribyte of the main xml
    /// \brief Dtmf::p_main
    /// \param main`s argc
    /// \param main`s argv
    /// \return 0/1
    ///
    int Dtmf::p_main(int argc, char **argv)
    {
        if (argc < 2) {
            return -1;
        } else {
            for(int i=0; i < argc; ++i) {
                if ((strcmp(argv[i], "-c") == 0) ||
                    (strcmp(argv[i], "--config"))) {
                    if (argv[i+1] != nullptr) {
                        utils::RecorderConfig::Instance().fastLoadFile(argv[i+1]);

                        using namespace utils;

                        PairList& ls = RecorderConfig::Instance().getTagPairs("Plugin");

                        for(int i=0; i < ls.count(); ++i) {
                            const MPair<QString, QString>& p = ls.at(i);
                            if (p.m_type2 == "dtmf") {
                                for(int ii=i; ii < ls.count(); ++ii) {
                                    if (ls.at(ii).m_type1 == "conf") {

                                        QString fname = ls.at(ii).m_type2.toStdString().data();
                                        RecorderConfig::Instance().fastLoadFile(fname);

                                        const MPair<QString, QString> &freq =
                                                RecorderConfig::Instance().getAttribPairFromTag("Config", "freq");
                                        const MPair<QString, QString> &frmSz =
                                                RecorderConfig::Instance().getAttribPairFromTag("Config", "frameSize");
                                        // better evaluation needed
                                        if (freq.m_type1 == "" || frmSz.m_type1 == "") {
                                            // set defaults
                                            Dtmf::s_freq = 102;
                                            Dtmf::s_frameSize = 160;
                                        } else {
                                            bool res = false;

                                            Dtmf::s_freq = freq.m_type2.toInt(&res);
                                            if (!res) {
                                                s_freq = 102;
                                            }

                                            Dtmf::s_frameSize = frmSz.m_type2.toInt(&res);
                                            if (!res) {
                                                s_frameSize = 160;
                                            }
                                            // makesure you exit here
                                            return 0;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        return 0;
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
           m_lock.lock();
           buffSize = m_sampleBuffer.data.count();
           m_lock.unlock();

           if (buffSize < Dtmf::s_frameSize) {
               continue;
           }
           m_lock.lock();

           for(int i=0; i < buffSize ; ++i){
               dbl.append(m_sampleBuffer.data.at(i));
           }
           m_sampleBuffer.data.clear();
           m_lock.unlock();
           if (dbl.isEmpty()) {
               continue;
           }
           int16_t* dtmf= new int16_t[Dtmf::s_frameSize];

           for(int it=0; it < dbl.count(); ++it) {

               for(int ii=0; ii < Dtmf::s_frameSize; ) {
                   if (dbl.isEmpty()) {
                       break;
                   }
                   utils::sample_data_t smpl = dbl.at(it);
                   for(uint32_t j=0; j < smpl.size; ++j) {
                       dtmf[ii++] = smpl.samples[j];
                   }
               }

               m_dtmfDetector->dtmfDetecting((INT16*)dtmf);

               dbl.clear();

               if (m_dtmfDetector->getIndexDialButtons() < 1) {
                   //printf("Error in detecting number of buttons\n");
                   continue;
               }

               int d = m_dtmfDetector->getIndexDialButtons();
               for(int ii = 0; ii < d; ++ii) {
                   if(m_dtmfDetector->getDialButtonsArray()[ii] != s_DialButtons[ii])
                   {
                   //    printf("Error of a detecting button \n");
                       continue;
                   } else {
                       printf("#[%d]\tWe got: [%c]\n", ii, s_DialButtons[ii]);
                   }
               }
                // clear the double buffer


           } // end if performing dtmf detection
           delete [] dtmf;

        } // busy loop
    }


    Dtmf::Dtmf(QThread *parent)
        : QThread(parent),
          m_isRunning(false),
          m_dtmfDetector(nullptr)
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
