#ifndef IZ_FFT_H
#define IZ_FFT_H
#include <QThread>
#include <QObject>
#include <QMutex>
#include <QList>


#include "plugin-interface.h"
#include "utils.h"

namespace plugin {
    namespace fft {
    struct data_t
    {
        float* data;
        unsigned long number_of_samples;
        unsigned int srate;
    };

    struct FFT
    {
        double pi;
        unsigned long fundamental_frequency;
        FFT();
        ~FFT();
        void ComplexFFT(data_t data, int sign);
    };

    class FFTPlugin : public QThread
    {
    public:
        static FFTPlugin& Instance();
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

    private:
        explicit FFTPlugin(QThread* parent = nullptr);
        virtual ~FFTPlugin();

        static FFTPlugin* s_inst;
        bool m_isRunning;
        FFT m_ftransform;
        QList<utils::sample_data_t> m_sampleBuffer;
        QMutex m_lock;
        // plugin stuff
        struct interface_t iface;
    };

    } // fft
} // plugin


#endif // IZ_FFT_H
