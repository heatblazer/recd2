#ifndef ALSAREC_H
#define ALSAREC_H

#include <alsa/asoundlib.h>
#include <stdio.h>
#include <stdlib.h>

// openal //
#include <AL/al.h>
#include <AL/alc.h>


#include "plugin-iface.h"
#include "thread.h"


namespace plugin {
    namespace alsarec {

    class AlRec
    {
    public:
        static void* worker(void* pArgs);
        static AlRec& Instance();
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
        explicit AlRec();
        ~AlRec();
        PThread m_thread;
        PMutex m_mutex;
        ALCdevice* p_device;
        static interface_t s_iface;
        static AlRec* s_inst;
    };

    } // alsarec
} // plugin


#endif // ALSAREC_H
