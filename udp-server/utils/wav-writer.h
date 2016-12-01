#ifndef WAVWRITER_H
#define WAVWRITER_H

// ansi c //
#include <stdio.h>
#include <stdint.h>

#include "types.h"
#include "wav-writer-iface.h"

/// the default wav header
/// \brief The wav_hdr_t struct


namespace iz {

/// a minimal and portable
/// wav file writer C library
/// \brief The Wav class
class Wav : public WavIface
{

public:
    Wav(const char* fname);
    virtual ~Wav();
    // in case we migrate to other file api we made these virtual
    // but provide some defailt implementation
    virtual bool open(unsigned slot);
    virtual void close();
    virtual bool isOpened() const ;
    virtual int write(short int data[], int len);
    virtual void* read();
    virtual void setupWave(int samples_per_sec=8000, int bits_per_sec=16, int riff_len=0,
                            int fmt_len=16, short audio_fmt=1,  short chann_cnt=1);
    virtual void write_hdr(int spf=44100, int bps=16, int rifflen=0, int fmtlen=16, short audfmt=1, short chans=1);    
    virtual const char* getFileName();
    virtual size_t getFileSize() const;
    virtual int     getSlot() const;
    virtual void renameFile(const char* oldname, const char* newname);

private:

protected:
    FILE*   m_file;
    wav_hdr_t m_header;
    char m_filename[64];
    bool m_isSetup;
    bool m_isOpened;
    bool m_requiresFlip;
    size_t m_maxSize;
    int m_slot; // slot to the file
};


} // iz


#endif // WAVWRITER_H
