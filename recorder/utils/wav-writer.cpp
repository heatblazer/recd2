#include "wav-writer.h"

#include <fcntl.h>
#include <string.h> // strcpy
#include <unistd.h>

static const char* THIS_FILE = "wav-writer.cpp";

namespace plugin {
namespace rec {

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
    } in = {0}, out = {0};

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
    } in = {0}, out = {0};

    in.i = input;
    // alwyas 2 byte swap
    out.c[0] = in.c[1];
    out.c[1] = in.c[0];

    return out.i;
}

Wav::Wav(const char *fname)
    : m_file(NULL),
      m_isSetup(false),
      m_isOpened(false),
      m_requiresFlip(false),
      m_maxSize(0),
      m_slot(-1)
{
    m_requiresFlip = isBigEndian();
    strcpy(m_filename, fname);
}

Wav::~Wav()
{
}

/// open a wav file
/// and write a wav header on it
/// \brief Wav::open
/// \param channel slot
/// \return
/// TODO: new logic here : write the filled header

bool Wav::open(unsigned slot)
{
    m_slot = slot;
    m_file = fopen(m_filename, "wb");
    if (m_file == NULL) {
        return m_isOpened;
    }

    // we have not called setup! Load some defaults !
    if (!m_isSetup) {
        // write default header
        setupWave();
    }

    fwrite(&m_header, sizeof(m_header), 1, m_file);
    fflush(m_file);
    // include header even though it`s agruably
    m_maxSize += 44;
    m_isOpened = true;
    return m_isOpened;

}

int Wav::getSlot() const
{
    return m_slot;
}

void Wav::renameFile(const char *oldname, const char *newname)
{
    rename(oldname, newname);
}


/// close the file and write
/// needed info to the header
/// as length and riff len
/// \brief Wav::close
///
void Wav::close()
{
    if (m_isOpened) {
        int file_len = ftell(m_file);

        // get the len from the offset
        int data_len = file_len - sizeof(struct wav_hdr_t);

        // move the fp to that position of the data len
        fseek(m_file, sizeof(struct wav_hdr_t) - sizeof(int), SEEK_SET);
        fwrite(&data_len, sizeof(data_len), 1, m_file);
        // this writes riff len to the position in
        // the header
        int riff_len = file_len - 8;
        fseek(m_file, 4, SEEK_SET);
        fwrite(&riff_len, sizeof(riff_len), 1, m_file);
        fclose(m_file);
        m_isOpened = false;
    }
    // else it`s not opened, nothing  to be done
}

bool Wav::isOpened() const
{
    return m_isOpened;
}

/// write data to wav file
/// \brief Wav::write
/// \param data - samples
/// \param len - size
/// \return  bytes writen if needed
///
int Wav::write(short data[], int len)
{
    fwrite(data, sizeof(short), len, m_file);

    // this will avoid checking the file size each time in
    // the system watcher, losing a bit of precision in bytes tho
    // in the final I`ll make sure ftell is called periodicly

    m_maxSize += (len * sizeof(short));
    return m_maxSize;
}

/// unimplemented since not needed
/// \brief Wav::read
/// \return
///
void *Wav::read()
{
    return nullptr;
}

///  new concept for setup a wav file before writing to it
///  better
/// \brief Wav::setupWave
/// \param samples_per_sec
/// \param bits_per_sec
/// \param riff_len
/// \param fmt_len
/// \param audio_fmt
/// \param chann_cnt
///
void Wav::setupWave(int samples_per_sec, int bits_per_sec, int riff_len,
                    int fmt_len, short audio_fmt, short chann_cnt)
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

    m_isSetup = true;
}

/// write the needed header to
/// a wav file
/// \brief Wav::write_hdr
/// \param spf
/// \param bps
/// \param rifflen
/// \param fmtlen
/// \param audfmt
/// \param chans
/// Old concept pending deletion ...
void Wav::write_hdr(int spf, int bps, int rifflen, int fmtlen, short audfmt, short chans)
{
    struct wav_hdr_t hdr;
    hdr.riff_tag[0] = 'R';
    hdr.riff_tag[1] = 'I';
    hdr.riff_tag[2] = 'F';
    hdr.riff_tag[3] = 'F';

    hdr.wav_tag[0] = 'W';
    hdr.wav_tag[1] = 'A';
    hdr.wav_tag[2] = 'V';
    hdr.wav_tag[3] = 'E';

    hdr.fmt_tag[0] = 'f';
    hdr.fmt_tag[1] = 'm';
    hdr.fmt_tag[2] = 't';
    hdr.fmt_tag[3] = ' ';

    hdr.data_tag[0] = 'd';
    hdr.data_tag[1] = 'a';
    hdr.data_tag[2] = 't';
    hdr.data_tag[3] = 'a';

    hdr.riff_len = rifflen;
    hdr.fmt_len = fmtlen;
    hdr.audio_format = audfmt;
    hdr.num_channels = chans;
    hdr.sample_rate = spf;
    hdr.byte_rate = spf * (bps/8);
    hdr.block_align = bps / 8;
    hdr.bits_per_sample = bps;
    hdr.data_len = 0;
    fwrite(&hdr, sizeof(hdr), 1, m_file);
    fflush(m_file);

}

const char *Wav::getFileName()
{
    return m_filename;
}

size_t Wav::getFileSize() const
{
    return m_maxSize;
}

} // rec
}  // plugin
