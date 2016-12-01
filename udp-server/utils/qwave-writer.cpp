#include "qwave-writer.h"

static const char* THIS_FILE = "qwave-writer.cpp";

namespace iz {

static inline bool isBigEndian(void)
{
    union {
        int16_t i;
        char c[sizeof(int16_t)];
    } integer;

    integer.i = 1;
    if (integer.i & (1 << 0) ) {
        // little
        return false;
    } else {
        return true;
    }
}

/// flip endiness 32 bit
/// on big endian machines
/// \brief flip32
/// \param input mostly big endian
/// \return flipped 4 bytes
///
static inline int32_t flip32(int32_t input)
{
    (void)flip32;

    union {
        int32_t i;
        char c[sizeof(int32_t)];
    } in, out;
    out.i = 0;
    in.i = input;
    // always 4 byte swap
    out.c[0] = in.c[3];
    out.c[1] = in.c[2];
    out.c[2] = in.c[1];
    out.c[3] = in.c[0];

    return out.i;
}

/// flip endiness 16 bit
/// \brief flip16
/// \param input moslty big endian
/// \return flipped 2 bytes
///
static inline int16_t flip16(int16_t input)
{
    union {
        int16_t i;
        char c[sizeof(int16_t)];
    } in, out;

    in.i = input;
    out.i = 0;
    // alwyas 2 byte swap
    out.c[0] = in.c[1];
    out.c[1] = in.c[0];

    return out.i;
}


bool QWav::isOpened() const
{
    return m_wav.isOpen();
}

/// unimplemented
/// \brief QWav::read
/// \return
///
void *QWav::read()
{
    return nullptr;
}

void QWav::setupWave(int samples_per_sec,
                     int bits_per_sec,
                     int riff_len,
                     int fmt_len,
                     short audio_fmt,
                     short chann_cnt)
{
    m_header.riff_tag[0] = 'R';
    m_header.riff_tag[1] = 'I';
    m_header.riff_tag[2] = 'F';
    m_header.riff_tag[3] = 'F';

    m_header.wav_tag[0] = 'W';
    m_header.wav_tag[1] = 'A';
    m_header.wav_tag[2] = 'V';
    m_header.wav_tag[3] = 'E';

    m_header.fmt_tag[0] = 'f';
    m_header.fmt_tag[1] = 'm';
    m_header.fmt_tag[2] = 't';
    m_header.fmt_tag[3] = ' ';

    m_header.data_tag[0] = 'd';
    m_header.data_tag[1] = 'a';
    m_header.data_tag[2] = 't';
    m_header.data_tag[3] = 'a';

    m_header.riff_len = riff_len;
    m_header.fmt_len = fmt_len;
    m_header.audio_format = audio_fmt;
    m_header.num_channels = chann_cnt;
    m_header.sample_rate = samples_per_sec;
    m_header.byte_rate = samples_per_sec * (bits_per_sec/8);
    m_header.block_align = bits_per_sec / 8;
    m_header.bits_per_sample = bits_per_sec;
    m_header.data_len = 0;

    m_setup = true;

}

/// each write appends bytes to the size var
/// so when I need to check size I won`t call
/// ftell or QFile::size() and that way I am
/// faster, since I don`t perform file seeks
/// \brief QWav::write
/// \param data
/// \param len
/// \return current file size
///
int QWav::write(short data[], int len)
{
    // hope that write returns ok data
    m_size += m_wav.write((char*) data, len);
    return m_size;
}

void QWav::close()
{
    if (m_wav.isOpen()) {

        qint64 file_len = m_wav.size();
        // get the len from the offset
        int data_len = file_len - sizeof(struct wav_hdr_t);

        // move the fp to that position of the data len
        m_wav.seek(sizeof(struct wav_hdr_t) - sizeof(int));
        m_wav.write((char*)&data_len);
        // this writes riff len to the position in
        // the header
        int riff_len = file_len - 8;
        m_wav.seek(4);
        m_wav.write((char*)&riff_len);
        m_wav.close();
    }
}

/// unused for the QWav
/// \brief QWav::open
/// \param perms - unused see open(QIODevice perms, int slot);
/// \return always false
///
bool QWav::open(unsigned slot)
{
    if (m_wav.exists()) {
        return false;
    }
    m_wav.setFileName(m_name);
    if (!m_wav.isOpen()) {
        m_wav.open(QIODevice::WriteOnly);
    }
    // we have not called setup! Load some defaults !
    if (!m_setup) {
        // write default header
        setupWave();
    }
    m_wav.write((char*)&m_header, sizeof(m_header));
    m_wav.flush();
    m_size += 44;
    m_slot = slot;
    return m_wav.isOpen();

}

QWav::QWav(const QString &fname)
    : m_name(fname),
      m_slot(-1),
      m_setup(false),
      m_size(0)
{

}

QWav::~QWav()
{

}

const char *QWav::getFileName()
{
    return m_name.toStdString().data();
}

size_t QWav::getFileSize() const
{
    return m_size;
}


int QWav::getSlot() const
{
    return m_slot;
}

void QWav::renameFile(const char *oldname, const char *newname)
{
    (void) oldname;
    m_wav.rename(QString(newname));
}

} // iz
