#ifndef ALSAREC_H
#define ALSAREC_H

#include <alsa/asoundlib.h>

#include <stdio.h>
#include <stdlib.h>

#include "plugin-iface.h"
#include "thread.h"


namespace plugin {
    namespace alsarec {

    class AlsaRec
    {
    public:
        static AlsaRec& Instance();
        static void* worker(void* pArgs);

        void createPlayback(const char* dev);
        void createCapture(const char* dev);
        void createPlayCapture(const char* pb, const char* cap);

        // interface
        static void init();
        static void deinit();
        static void copy(const void* src, void* dest, int len);
        static int put_ndata(void* data, int len);
        static int put_data(void* data);
        static void* get_data(void);
        static int main_proxy(int argc, char** argv);
        static struct interface_t* getSelf(void);

    private:
        AlsaRec();
        ~AlsaRec();

        snd_pcm_uframes_t   m_frames ;
        unsigned int        m_rate;
        bool                m_isOk;
        int                 m_dir;
        struct {
            snd_pcm_t* cap_handle;
            snd_pcm_hw_params_t* hw_params;
            snd_pcm_format_t format;
        } m_alsa;

        PThread* m_athread;
        PMutex   m_mutex;
        static interface_t s_iface;
        static AlsaRec* s_inst;
    };

    } // alsarec
} // plugin


#endif // ALSAREC_H
