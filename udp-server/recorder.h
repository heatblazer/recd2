#ifndef RECORDER_H
#define RECORDER_H

// qt - paretn //
#include <QFileSystemWatcher> // monitor wav files for size for the hotswap
#include <QObject>
#include <QTimer> // hotswap interval

#include "types.h"
#include "utils/recorder-config.h"
#include "utils/json-writer.h"

namespace iz {

class WavIface;

class Recorder : public QObject
{
    Q_OBJECT // this class may be emiter
public:
    explicit Recorder(QObject* parent=nullptr);
    virtual ~Recorder(); // we may inherit it too
    // will use later the num_channels when concept is more clear
    bool init();
    void deinit();
    WavIface *getWavByName(const QString& fname);


private:
    bool setupWavFiles();

signals:
    void recordedBytes(uint32_t bytes);

public slots:
    void record(QQueue<udp_data_t> &packets);
    void record(const udp_data_t& data);
    void record(const tcp_data_t& data);

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
    JsonWriter m_jsonWritter;

};

} // iz

#endif // RECORDER_H
