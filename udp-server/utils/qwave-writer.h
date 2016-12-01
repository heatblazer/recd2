#ifndef QWAVEWRITER_H
#define QWAVEWRITER_H

// qt stuff //
#include <QFile>

#include "types.h"
#include "wav-writer-iface.h"

namespace iz {
/// newer concept for wave writer using QT
/// \brief The QWave class
///
class QWav : public WavIface
{
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
    explicit QWav(const QString& fname);
    virtual ~QWav();
    virtual bool open(unsigned slot);
    virtual void close();
    virtual int write(short int data[], int len);
    virtual void setupWave(int samples_per_sec=8000, int bits_per_sec=16, int riff_len=0,
                           int fmt_len=16, short audio_fmt=1,  short chann_cnt=1);
    virtual void* read();
    virtual bool isOpened() const;

    const char*  getFileName();
    size_t  getFileSize() const;
    virtual int getSlot() const;
    virtual void renameFile(const char* oldname, const char* newname);

private:
    QString m_name;
    QFile m_wav;
    int   m_slot;
    bool  m_setup;
    size_t m_size;
    wav_hdr_t m_header;

};


} // iz


#endif // QWAVEWRITER_H
