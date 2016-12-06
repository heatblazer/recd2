#include "alsarec.h"

// ansi c //
#include <stdio.h>
#include <stdlib.h>

// alloc //
#include <alloca.h> // automatic freed memory

#include "thread.h"

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

        while (arec->m_athread->isRunning()) {
            if ((err = snd_pcm_readi(arec->m_alsa.cap_handle,
                                     buffer, arec->m_frames)) != arec->m_frames) {
                fprintf(stderr, "failed to read from device: (%s)\n",
                        snd_strerror(err));
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

        fprintf(stdout, "Initializing alsa...\n");
        int err = 0;
        AlsaRec* aref = &Instance();
        if ((err = snd_pcm_open(&aref->m_alsa.cap_handle, "hw:0", SND_PCM_STREAM_CAPTURE, 0) < 0)) {
            fprintf(stderr, "can not open sound device: hw:0 (%s)\n",
                    snd_strerror(err));
            return ;
        }
        fprintf(stdout, "audio device opened!\n");

        if ((err = snd_pcm_hw_params_malloc(&aref->m_alsa.hw_params)) < 0) {
            fprintf(stderr, "cannot allocate hardware param struct: (%s)\n",
                    snd_strerror(err));
            return ;
        }

        fprintf(stdout,"hardware params allocated\n");
        if ((err = snd_pcm_hw_params_any(aref->m_alsa.cap_handle,
                                         aref->m_alsa.hw_params)) < 0) {
            fprintf(stderr, "failed to hardware structure: (%s)\n",
                    snd_strerror(err));
            return;
        }


        fprintf(stdout, "hardware params allocated!\n");
        if ((err = snd_pcm_hw_params_set_access(aref->m_alsa.cap_handle,
                                                aref->m_alsa.hw_params,
                                                SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
            fprintf(stderr, "cannot set access type (%s) \n",
                    snd_strerror(err));
            return;
        }

        fprintf(stdout, "hw params access setted\n");

        if ((err = snd_pcm_hw_params_set_format(
                 aref->m_alsa.cap_handle,
                 aref->m_alsa.hw_params,
                 aref->m_alsa.format)) < 0) {
            fprintf(stderr, "cannot set sample format: (%s)\n",
                    snd_strerror(err));
            return;
        }

        fprintf(stdout, "hw formats set ok!\n");


        if ((err = snd_pcm_hw_params_set_rate_near(
                 aref->m_alsa.cap_handle,
                 aref->m_alsa.hw_params, &aref->m_rate, 0)) < 0) {
            fprintf(stderr, "cannot set sample rate: (%s)\n",
                    snd_strerror(err));
            return;
        }

        if ((err = snd_pcm_hw_params_set_channels(
                 aref->m_alsa.cap_handle,
                 aref->m_alsa.hw_params, 2)) < 0) {
            fprintf(stderr, "cannot set channel count (%s)\n",
                    snd_strerror(err));
            return;
        }

        fprintf(stdout, "hw params channels setted\n");

        if ((err = snd_pcm_hw_params(
                 aref->m_alsa.cap_handle,
                 aref->m_alsa.hw_params)) < 0) {
            fprintf(stderr, "cannot set params: (%s)\n",
                    snd_strerror(err));
            return;
        }

        fprintf(stdout, "hardware params set ok\n");
        snd_pcm_hw_params_free(aref->m_alsa.hw_params);
        fprintf(stdout, "hardware params freed\n");

        if ((err = snd_pcm_prepare(aref->m_alsa.cap_handle)) < 0) {
            fprintf(stderr, "cannot prepare audio interface: (%s)\n",
                    snd_strerror(err));
            return;
        }

        fprintf(stdout, "audio interface prepared... starting up...\n");
        aref->m_isOk = true;

        aref->m_athread = new PThread;
        aref->m_athread->create(128 * 1024, aref, AlsaRec::worker, 20);
        aref->m_athread->setName("alsa-thread");

    }

    /// deinit alsa
    /// \brief AlsaRec::deinit
    ///
    void AlsaRec::deinit()
    {
        fprintf(stdout, "Sound device closed...\n");
        AlsaRec* aref = &Instance();
        aref->m_athread->setRunning(false);
        aref->m_athread->join();
        aref->m_athread->yield();
        snd_pcm_close(aref->m_alsa.cap_handle);

    }

    void AlsaRec::copy(const void *src, void *dest, int len)
    {
        (void) src; (void) dest; (void) len;
    }

    int AlsaRec::put_ndata(void *data, int len)
    {
        printf("AlsaRec: put data to ");
        if (s_iface.nextPlugin != nullptr) {
            puts("next plugin.");
            s_iface.nextPlugin->put_ndata(data, len);
        } else {
            puts("no one.");
        }
        return 0;

    }

    int AlsaRec::put_data(void *data)
    {
        printf("AlsaRec: put data to ");
        if (s_iface.nextPlugin != nullptr) {
            puts("next plugin.");
            s_iface.nextPlugin->put_data(data);
        } else {
            puts("no one.");
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

    pif->none = nullptr;
    pif->nextPlugin = nullptr;

    return plugin::alsarec::AlsaRec::Instance().getSelf();
}
