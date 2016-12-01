#ifndef CONFIG_H
#define CONFIG_H

// qt stuff //
#include <QFile>

namespace iz {

// simple cfg file reader for now
// no xml walkers and complex stuff,
// for fast editing a simple .cfg file
class WavConfig
{
    //int spf, int bps, int rifflen, int fmtlen, short audfmt, short chans
public:
    enum Params {
        SAMPLES_PER_FRAME,
        BITS_PER_SEC,
        RIFF_LEN,
        AUD_FORMAT,
        CHANNELS,
        SIZE
    };

    explicit WavConfig(const QString& fname);
    virtual ~WavConfig();
    int getAttribute(Params atrib);

private:
    struct {
        QString name;
        QFile   file;
    } m_file;
    int m_attribs[Params::SIZE];
};

}  // iz


#endif // CONFIG_H
