#include "alsarec.h"

#include <QList>

#include <stdio.h>
#include <stdlib.h>

#include "thread.h"
#include "recorder-config.h"
#include "ipc-msg.h"

//static const char* THIS_FILE = "alsarec.cpp";


namespace plugin {
    namespace alsarec {

    struct sample_data_t
    {
        short* samples;
        uint32_t size;
    };

    AlRec* AlRec::s_inst = nullptr;
    interface_t AlRec::s_iface;

    int AlRec::main_proxy(int argc, char **argv)
    {
        if (argc < 2) {
            return -1;
        } else {
            for(int i=0; i < argc; ++i) {
                if ((strcmp(argv[i], "-c") == 0) ||
                    (strcmp(argv[i], "--config"))) {
                    if (argv[i+1] != nullptr) {
                        utils::RecorderConfig::Instance().fastLoadFile(argv[i+1]);
                    }
                }
            }
        }
        return 0;
    }


    void *AlRec::worker(void *pArgs)
    {
        AlRec* oalrec = (AlRec*) pArgs;
        ALint sample;
        ALbyte buffer[1024] = {0};

        while (oalrec->m_thread.isRunning()) {
            alcGetIntegerv(oalrec->p_device, ALC_CAPTURE_SAMPLES,
                           (ALCsizei)sizeof(ALshort), &sample);
            alcCaptureSamples(oalrec->p_device, (ALCvoid*) buffer, sample);
            {
                sample_data_t smpl = {0, 0};
                smpl.samples = new short[1024];
                smpl.size = 1024;

                for(int i=0; i < 1024; ++i) {
                    smpl.samples[i] = (short) sample;
                }
                QList<sample_data_t> sd;
                sd.append(smpl);
                put_data((QList<sample_data_t>*)&sd);
            }
        }

        alcCaptureStop(oalrec->p_device);
        alcCaptureCloseDevice(oalrec->p_device);
        return nullptr;
    }

    AlRec &AlRec::Instance()
    {
        if (s_inst == nullptr) {
            s_inst = new AlRec;
        }
        return *s_inst;
    }

    void AlRec::init()
    {
        AlRec* arec = &Instance();
        alGetError();
        arec->p_device = alcCaptureOpenDevice(NULL, 8000, AL_FORMAT_MONO8, 1024);

        arec->m_mutex.init();
        arec->m_thread.create(128, arec, AlRec::worker, 20);
        arec->m_thread.setName("sound thread");
    }

    void AlRec::deinit()
    {
        AlRec* arec = &Instance();
        arec->m_thread.join();
        arec->m_thread.yield();
        arec->m_mutex.destroy();
    }

    void AlRec::copy(const void *src, void *dest, int len)
    {
        (void) src; (void) dest; (void) len;
    }

    int AlRec::put_ndata(void *data, int len)
    {
        if (s_iface.nextPlugin != nullptr) {
            s_iface.nextPlugin->put_ndata(data, len);
        }
        return 0;
    }

    int AlRec::put_data(void *data)
    {
        if (s_iface.nextPlugin != nullptr) {
            s_iface.nextPlugin->put_data(data);
        }
        return 0;
    }

    void *AlRec::get_data()
    {
        return nullptr;
    }

    void AlRec::setName(const char *name)
    {
        strncpy(s_iface.name, name, 256);
    }

    const char *AlRec::getName()
    {
        return s_iface.name;
    }

    interface_t *AlRec::getSelf()
    {
        return &s_iface;
    }

    AlRec::AlRec()
    {

    }

    AlRec::~AlRec()
    {

    }

    } // alsarec
} // plugin

interface_t* get_interface()
{
    interface_t* pif = plugin::alsarec::AlRec::Instance().getSelf();

    pif->init = &plugin::alsarec::AlRec::init;
    pif->deinit = &plugin::alsarec::AlRec::deinit;
    pif->copy = &plugin::alsarec::AlRec::copy;
    pif->put_data = &plugin::alsarec::AlRec::put_data;
    pif->put_ndata = &plugin::alsarec::AlRec::put_ndata;
    pif->main_proxy = &plugin::alsarec::AlRec::main_proxy;
    pif->get_data  = &plugin::alsarec::AlRec::get_data;
    pif->getSelf = &plugin::alsarec::AlRec::getSelf;
    pif->setName = &plugin::alsarec::AlRec::setName;
    pif->getName = &plugin::alsarec::AlRec::getName;
    pif->nextPlugin = nullptr;

    return plugin::alsarec::AlRec::Instance().getSelf();
}
