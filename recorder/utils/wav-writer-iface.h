#ifndef WAVWRITERIFACE_H
#define WAVWRITERIFACE_H

#include <unistd.h>

namespace plugin {
namespace rec {
/// typical wav interface
/// \brief The WavIface class
///
class WavIface
{
public:
    /// opens a wav file
    /// \brief open
    /// \param channel slot
    /// \return true on success, false else
    ///
    virtual bool open(unsigned slot) = 0;

    /// closes the file
    /// \brief close
    ///
    virtual void close() = 0;

    /// write N samples of data to file
    /// \brief write
    /// \param data - samples pointer
    /// \param len - num samples
    /// \return samples written
    ///
    virtual int write(short int data[], int len) = 0;

    /// setup a wav header
    /// \brief setupWave
    /// \param samples_per_sec
    /// \param bits_per_sec
    /// \param riff_len
    /// \param fmt_len
    /// \param audio_fmt
    /// \param chann_cnt
    ///
    virtual void setupWave(int samples_per_sec,
                           int bits_per_sec,
                           int riff_len,
                           int fmt_len,
                           short audio_fmt,
                           short chann_cnt) = 0;

    /// read data from file - not used anywhere
    /// \brief read
    /// \return data from the file
    ///
    virtual void* read() = 0;

    /// return OK if file is opened
    /// \brief isOpened
    /// \return true if opened, otherwise false
    ///
    virtual bool isOpened() const = 0;

    /// get name of file
    /// \brief getFileName
    /// \return filename
    ///
    virtual  const char* getFileName() = 0;

    /// get filesize in bytes
    /// \brief getFileSize
    /// \return filesizew
    ///
    virtual size_t getFileSize() const = 0;

    /// get channel slot associated with this file
    /// \brief getSlot
    /// \return channel slot
    ///
    virtual int     getSlot() const = 0;

    /// rename a filename
    /// \brief renameFile - renames a filename by a given pattern
    /// \param oldname
    /// \param newname
    ///
    virtual void renameFile(const char* oldname, const char* newname) = 0;

};

} // rec
} // plugin

#endif // WAVWRITERIFACE_H
