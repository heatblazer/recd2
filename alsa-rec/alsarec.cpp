#include "alsarec.h"

// ansi c //
#include <stdio.h>
#include <stdlib.h>

// alloc //
#include <alloca.h> // automatic freed memory

// thread //
#include "thread.h"

// utils //
#include "logger.h"

static const char* THIS_FILE = "alsarec.cpp";
namespace plugin {
    namespace alsarec {

    interface_t AlsaRec::s_iface;
    AlsaRec* AlsaRec::s_inst = nullptr;

    AlsaRec::AlsaRec()
        : m_frames(16),
          m_rate(8000),
          m_isOk(false),
          m_alsa{nullptr, nullptr, SND_PCM_FORMAT_S16_LE},
          m_athread(nullptr)
    {
    }

    AlsaRec::~AlsaRec()
    {
    }

    AlsaRec &AlsaRec::Instance()
    {
        if (s_inst == nullptr) {
            s_inst = new AlsaRec;
        }

        return *s_inst;
    }

    /// alsa worker
    /// \brief AlsaRec::worker
    /// \param pArgs - this AlsaRec
    /// \return NULL on fails
    ///
    void *AlsaRec::worker(void *pArgs)
    {
        AlsaRec* arec = (AlsaRec*) pArgs;
        if (!arec->m_isOk) {
            return NULL;
        }
        char* buffer = NULL;

        // will be freed at the end of worker(...)
        buffer = (char*) alloca(arec->m_frames * snd_pcm_format_width(arec->m_alsa.format) / 8 *2);

        if (!buffer) {
            return NULL;
        }
        int err = -1;
        static char err_msg[256] = {0};
        while (arec->m_athread->isRunning()) {

            if ((err = snd_pcm_readi(arec->m_alsa.cap_handle,
                                     buffer, arec->m_frames)) != arec->m_frames) {
                snprintf(err_msg, sizeof(err_msg),
                         "failed to read from device: (%s)\n",
                        snd_strerror(err));
            //    utils::Logger::Instance().logMessage(THIS_FILE, err_msg);
            } else {
                snd_pcm_prepare(arec->m_alsa.cap_handle);
                put_ndata(buffer, arec->m_frames);
            }
        }

        snd_pcm_drain(arec->m_alsa.cap_handle);

        return NULL;
    }

    /// perform alsa setup here
    /// \brief AlsaRec::init
    ///
    void AlsaRec::init()
    {
        static char msg[256] = {0};

        snprintf(msg, sizeof(msg), "Initializing alsarec...\n");
      //  utils::Logger::Instance().logMessage(THIS_FILE, msg);
        int err = 0;
        AlsaRec* aref = &Instance();

        if ((err = snd_pcm_open(&aref->m_alsa.cap_handle, "hw:0", SND_PCM_STREAM_CAPTURE,
                                0) < 0)) {
            snprintf(msg, sizeof(msg), "can not open sound device: hw:0 (%s)\n",
                    snd_strerror(err));
        //    utils::Logger::Instance().logMessage(THIS_FILE, msg);
            return ;
        }
        snprintf(msg, sizeof(msg), "audio device opened!\n");
      //  utils::Logger::Instance().logMessage(THIS_FILE, msg);

        if ((err = snd_pcm_hw_params_malloc(&aref->m_alsa.hw_params)) < 0) {
            snprintf(msg, sizeof(msg), "cannot allocate hardware param struct: (%s)\n",
                    snd_strerror(err));
       //     utils::Logger::Instance().logMessage(THIS_FILE, msg);
            return ;
        }

        snprintf(msg, sizeof(msg), "hardware params allocated\n");
        utils::Logger::Instance().logMessage(THIS_FILE, msg);

        if ((err = snd_pcm_hw_params_any(aref->m_alsa.cap_handle,
                                         aref->m_alsa.hw_params)) < 0) {
            snprintf(msg, sizeof(msg), "failed to hardware structure: (%s)\n",
                    snd_strerror(err));
            utils::Logger::Instance().logMessage(THIS_FILE, msg);
            return;
        }


      //  snprintf(msg,sizeof(msg),  "hardware params allocated!\n");
        utils::Logger::Instance().logMessage(THIS_FILE, msg);

        if ((err = snd_pcm_hw_params_set_access(aref->m_alsa.cap_handle,
                                                aref->m_alsa.hw_params,
                                                SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
            snprintf(msg, sizeof(msg), "cannot set access type (%s) \n",
                    snd_strerror(err));
        //    utils::Logger::Instance().logMessage(THIS_FILE, msg);
            return;
        }

        snprintf(msg, sizeof(msg), "hw params access setted\n");
      //  utils::Logger::Instance().logMessage(THIS_FILE, msg);

        if ((err = snd_pcm_hw_params_set_format(
                 aref->m_alsa.cap_handle,
                 aref->m_alsa.hw_params,
                 aref->m_alsa.format)) < 0) {
            snprintf(msg, sizeof(msg), "cannot set sample format: (%s)\n",
                    snd_strerror(err));
        //    utils::Logger::Instance().logMessage(THIS_FILE, msg);
            return;
        }

        snprintf(msg, sizeof(msg), "hw formats set ok!\n");
        utils::Logger::Instance().logMessage(THIS_FILE, msg);

        if ((err = snd_pcm_hw_params_set_rate_near(
                 aref->m_alsa.cap_handle,
                 aref->m_alsa.hw_params, &aref->m_rate, 0)) < 0) {
            snprintf(msg, sizeof(msg), "cannot set sample rate: (%s)\n",
                    snd_strerror(err));
        //    utils::Logger::Instance().logMessage(THIS_FILE, msg);
            return;
        }

        if ((err = snd_pcm_hw_params_set_channels(
                 aref->m_alsa.cap_handle,
                 aref->m_alsa.hw_params, 2)) < 0) {
            snprintf(msg, sizeof(msg), "cannot set channel count (%s)\n",
                    snd_strerror(err));
        //    utils::Logger::Instance().logMessage(THIS_FILE, msg);
            return;
        }

        snprintf(msg, sizeof(msg), "hw params channels setted\n");
       // utils::Logger::Instance().logMessage(THIS_FILE, msg);
        if ((err = snd_pcm_hw_params(
                 aref->m_alsa.cap_handle,
                 aref->m_alsa.hw_params)) < 0) {
            snprintf(msg, sizeof(msg), "cannot set params: (%s)\n",
                    snd_strerror(err));
       //     utils::Logger::Instance().logMessage(THIS_FILE, msg);
            return;
        }

        snprintf(msg, sizeof(msg), "hardware params set ok\n");
       // utils::Logger::Instance().logMessage(THIS_FILE, msg);

        snd_pcm_hw_params_free(aref->m_alsa.hw_params);

        snprintf(msg, sizeof(msg), "hardware params freed\n");
      //  utils::Logger::Instance().logMessage(THIS_FILE, msg);

        if ((err = snd_pcm_prepare(aref->m_alsa.cap_handle)) < 0) {
            snprintf(msg, sizeof(msg), "cannot prepare audio interface: (%s)\n",
                    snd_strerror(err));
      //      utils::Logger::Instance().logMessage(THIS_FILE, msg);
            return;
        }

        snprintf(msg, sizeof(msg), "audio interface prepared... starting up...\n");
      //  utils::Logger::Instance().logMessage(THIS_FILE, msg);
        aref->m_isOk = true;

        aref->m_mutex.init();
        aref->m_athread = new PThread;
        aref->m_athread->create(128 * 1024, aref, AlsaRec::worker, 20);
        aref->m_athread->setName("alsa-thread");

    }

    /// deinit alsa
    /// \brief AlsaRec::deinit
    ///
    void AlsaRec::deinit()
    {
        static char msg[64] = {0};
        snprintf(msg, sizeof(msg), "Sound device closed...\n");
        AlsaRec* aref = &Instance();
        aref->m_athread->setRunning(false);
        aref->m_athread->join();
        aref->m_athread->yield();
        snd_pcm_close(aref->m_alsa.cap_handle);
        if (aref->m_athread != nullptr) {
            delete aref->m_athread;
        }

        aref->m_mutex.destroy();

    }

    void AlsaRec::copy(const void *src, void *dest, int len)
    {
        (void) src; (void) dest; (void) len;
    }

    int AlsaRec::put_ndata(void *data, int len)
    {
        if (s_iface.nextPlugin != nullptr) {
            s_iface.nextPlugin->put_ndata(data, len);
        } else {
        }
        return 0;

    }

    int AlsaRec::put_data(void *data)
    {
        if (s_iface.nextPlugin != nullptr) {
            s_iface.nextPlugin->put_data(data);
        } else {
        }
        return 0;
    }

    void *AlsaRec::get_data()
    {
        return nullptr;
    }

    int AlsaRec::main_proxy(int argc, char **argv)
    {
        (void) argc;
        (void) argv;
        printf("alsarec main: \n");
        return 0;
    }

    interface_t *AlsaRec::getSelf()
    {
        return &s_iface;
    }

    } // alsarec
} // plugin

interface_t* get_interface()
{
    interface_t* pif = plugin::alsarec::AlsaRec::Instance().getSelf();

    pif->init = &plugin::alsarec::AlsaRec::init;
    pif->deinit = &plugin::alsarec::AlsaRec::deinit;
    pif->copy = &plugin::alsarec::AlsaRec::copy;
    pif->put_data = &plugin::alsarec::AlsaRec::put_data;
    pif->put_ndata = &plugin::alsarec::AlsaRec::put_ndata;
    pif->main_proxy = &plugin::alsarec::AlsaRec::main_proxy;
    pif->get_data  = &plugin::alsarec::AlsaRec::get_data;
    pif->getSelf = &plugin::alsarec::AlsaRec::getSelf;
    pif->nextPlugin = nullptr;

    return plugin::alsarec::AlsaRec::Instance().getSelf();
}
