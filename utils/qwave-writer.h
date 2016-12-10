#ifndef QWAVEWRITER_H
#define QWAVEWRITER_H

#include <QFile>
#include <QThread>
#include <QMutex>
#include <QQueue>

#include "types.h"
#include "wav-writer-iface.h"

namespace utils {

/// newer concept for wave writer using QT
/// \brief The QWave class
///
class QWav : public QObject, public WavIface
{
    // this class is emiter
    Q_OBJECT
public:
    enum OpenMode {
        NotOpen = 0x0000,
        ReadOnly = 0x0001,
        WriteOnly = 0x0002,
        ReadWrite = ReadOnly | WriteOnly,
        Append = 0x0004,
        Truncate = 0x0008,
        Text = 0x0010,
        Unbuffered = 0x0020
    };
    explicit QWav(const QString& fname, QObject* parent=nullptr);
    virtual ~QWav();

    virtual bool open(unsigned slot);
    virtual void close();
    virtual int write(short int data[], int len);
    virtual void setupWave(int samples_per_sec=8000, int bits_per_sec=16, int riff_len=0,
                           int fmt_len=16, short audio_fmt=1,  short chann_cnt=1);
    virtual void* read();
    virtual bool isOpened() const;

    const char*  getFileName();
    size_t  getFileSrecde() const;
    virtual int getSlot() const;
    virtual void renameFile(const char* oldname, const char* newname);

signals:
    void fileSrecdeChanged(utils::QWav* this_file);
    void fileSrecdeChanged(const int slot);

private:
    QString m_name;
    QFile m_wav;
    int   m_slot;
    bool  m_setup;
    bool m_isOpened;
    size_t m_srecde;
    wav_hdr_t m_header;

};

} // utils

#endif // QWAVEWRITER_H
