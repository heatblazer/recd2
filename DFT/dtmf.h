#ifndef DTMF_H
#define DTMF_H

#include "plugin-interface.h"
#include "utils.h"

// lib for dtmf //
#include "DtmfDetector.hpp"

namespace plugin {
    namespace dtmf {

    class Dtmf : public QThread
    {
    public:
        static Dtmf& Instance();
        // plugin stuff
        static void init();
        static void deinit(void);
        static void copy(const void* src, void* dst, int len);
        static int put_data(void* data);
        static int put_ndata(void* data, int len);
        static void* get_data(void);
        static void setName(const char* name);
        static const char* getName(void);
        static int p_main(int argc, char** argv);
        static struct interface_t* getSelf(void);

        // QThread overriding
        virtual void run() Q_DECL_OVERRIDE;

        int16_t hwm(int16_t val);
        int16_t m_peek ;
    private:
        explicit Dtmf(QThread* parent = nullptr);
        ~Dtmf();

        static Dtmf* s_inst;
        bool m_isRunning;
        DtmfDetector m_dtmfDetector;
        struct {
            bool is_busy;
            QList<utils::sample_data_t> data;
        } m_sampleBuffer;
        QMutex m_lock;
        // plugin stuff
        struct interface_t iface;
    };

    } // dtmf
} // plugin




#endif // DTMF_H
