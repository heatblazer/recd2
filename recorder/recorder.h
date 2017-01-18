#ifndef RECORDER_H
#define RECORDER_H
#include "utils.h"

#include <QObject>
#include <QTimer> // hotswap interval
#include <QThread>
#include <QMutex>
#include <QQueue>

#include "utils.h"
#include "plugin-iface.h"
#include "wav-writer-iface.h"

namespace plugin {
    namespace rec {

    struct udp_data_t
    {
        uint32_t    counter;
        uint8_t     null_bytes[32];
        int16_t    data[32][16];
    };


   /// threadable
    /// \brief The Recorder class
    ///
    class Recorder : public QThread
    {
        Q_OBJECT // this class may be emiter
    public:
        // will use later the num_channels when concept is more clear
        static Recorder& Instance();
        static void init();
        static void deinit(void);
        static void copy(const void* src, void* dst, int len);
        static int put_data(void* data);
        static int put_ndata(void* data, int len);
        static void* get_data(void);
        static void setName(const char* name);
        static const char* getName(void);
        static int main_proxy(int argc, char** argv);
        static struct interface_t* getSelf(void);

        utils::WavIface *getWavByName(const QString& fname);

        // threads stuff
        void run() Q_DECL_OVERRIDE;
        void startRecorder();
        void stopRecoder();
        void record(QList<utils::sample_data_t> sd);
    private:
        explicit Recorder(QThread *parent=nullptr);
        virtual ~Recorder(); // we may inherit it too
        bool setupWavFiles();

    signals:
        void recordedBytes(uint32_t bytes);

    private slots:
        // hot swap - time based
        void hotSwapFiles();

        // try swap on size based
        void pollHotSwap();

    private:
        // 128  chans max - I can use Wav** m_wavs but
        // I`ll just not use the rest since double ptr is
        // not allowed...

        // abstracted!!!
        utils::WavIface* m_wavs[128];
        // hotswap
        QTimer      m_hotswap; // timer based
        int         m_maxChans;
        bool        m_sizeBased;
        uint32_t    m_maxFileSize;
        static uint32_t s_UID;

        struct {
            int     samples_per_sec;
            int     bits_per_sec;
            int     riff_len;
            int     fmt_len;
            short   audio_fmt;
            short   chann_cnt;
        } m_wavParams;

        QString m_directory;

        static struct interface_t iface;
        static Recorder* s_inst;

        // concurent stuff
        struct {
            QMutex mutex;
            QQueue<QList<utils::sample_data_t> >buffer;
            bool running;
            unsigned long speed; // sleep interval
        } m_thread;
    };

    } // rec
} // plugin

#endif // RECORDER_H
