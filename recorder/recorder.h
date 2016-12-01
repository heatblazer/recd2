#ifndef RECORDER_H
#define RECORDER_H

// qt - paretn //
#include <QFileSystemWatcher> // monitor wav files for size for the hotswap
#include <QObject>
#include <QTimer> // hotswap interval

//  local hdrs //
#include "utils/recorder-config.h"
#include "plugin-iface.h"

namespace plugin {
namespace rec {

struct udp_data_t
{
    uint32_t    counter;
    uint8_t     null_bytes[32];
    int16_t    data[32][16];
};

class WavIface;

class Recorder : public QObject
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
    static int main_proxy(int argc, char** argv);
    static struct interface_t* getSelf(void);

    WavIface *getWavByName(const QString& fname);

private:
    explicit Recorder(QObject* parent=nullptr);
    virtual ~Recorder(); // we may inherit it too
    bool setupWavFiles();

signals:
    void recordedBytes(uint32_t bytes);

public slots:
    void record(QQueue<udp_data_t> &packets);
    void record(const udp_data_t& data);

private slots:
    // hot swap - time based
    void hotSwapFiles();

    // hot spaw filesystem watcher
    void performHotSwap(const QString& file);

private:
    // 128  chans max - I can use Wav** m_wavs but
    // I`ll just not use the rest since double ptr is
    // not allowed...

    // abstracted!!!
    WavIface* m_wavs[128];
    int m_maxChans;
    // hotswap
    QTimer m_hotswap; // timer based

    QFileSystemWatcher m_filewatcher; // size based

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
};

} // rec
} // plugin

#endif // RECORDER_H
