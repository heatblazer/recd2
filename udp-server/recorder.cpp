#include "recorder.h"

// remove later
#include <iostream>

// qt //
#include <QDir>

// parent app //
#include "sapplication.h"

// utils //
#include "unix/date-time.h"
#include "utils/logger.h"
#include "utils/wav-writer.h"

static const char* THIS_FILE = "recorder.cpp";

/// aux function to check for digits
/// \brief is_digit
/// \param str digit str
/// \return digit str
///
static inline const char* is_digit(const char* str) {
#define DIGIT(c) ((c >= '0') && (c <= '9'))
    static char digit[10] = {0};
    int size = 0;
    while (DIGIT(*str) && size < 10) {
        digit[size] = *str;
        size++;
        str++;
    }
#undef DIGIT
    return digit;
}
namespace iz {

struct udp_data_t
{
    uint32_t    counter;
    uint8_t     null_bytes[32];
#if (REQUIRE_FLIP_CHANNS_SAMPLES)
    int16_t    data[16][32];
#else
    int16_t    data[32][16];
#endif
};

struct tcp_data_t
{
    int16_t data[128];
};


uint32_t Recorder::s_UID = 0;

Recorder::Recorder(QObject *parent)
    : QObject(parent),
      m_maxChans(0),
      m_maxFileSize(0)
{
    for(int i=0; i < 128; ++i) {
        m_wavs[i] = nullptr;
    }
}

// handle with care the opened files
Recorder::~Recorder()
{
}

/// TODO: config file
/// will apply timestapm from the config later
/// \brief Recorder::init
/// \return true by default , false for future if something happens
bool Recorder::init()
{
    char init_msg[256] = {0};
    char buff[256]={0};
    bool res = true;

    Logger::Instance().logMessage(THIS_FILE, "Initializing recorder...\n");
    const MPair<QString, QString>& dir =
            RecorderConfig::Instance()
            .getAttribPairFromTag("Paths", "records");

    if (dir.m_type2 != "") {
        if (!QDir(dir.m_type2).exists()) {
            QDir().mkdir(dir.m_type2);
        }
    }

    // setup channels
    const MPair<QString, QString> max =
            RecorderConfig::Instance().getAttribPairFromTag("Channels", "count");
    if (max.m_type1 == "") {
        m_maxChans = 32;
    } else {
        bool res = false;
        m_maxChans = max.m_type2.toInt(&res);
        if (!res || m_maxChans > 127 || m_maxChans <= 0) {
            // precatuions !!!
            m_maxChans = 32;
        }
    }

    for(int i=0; i < m_maxChans; ++i) {
        s_UID++;
        if (dir.m_type1 != "") {
            snprintf(buff, sizeof(buff), "%s/%d-%d-%s.wav",
                    dir.m_type2.toStdString().data(),
                    i,
                    s_UID,
                    DateTime::getTimeString());
            m_directory = dir.m_type2;
        } else {
            snprintf(buff, sizeof(buff), "%d-%d-%s.wav", i,
                    s_UID,
                    DateTime::getTimeString());
            m_directory.clear();
        }
        m_wavs[i] = new Wav(buff);
    }
    // open files when everything is ok and setup
    res &= setupWavFiles();

    for(int i=0; i < m_maxChans; ++i) {
        res &= m_wavs[i]->open(i);
        if (res) {
            m_filewatcher.addPath(m_wavs[i]->getFileName());
        }
    }

    const MPair<QString, QString>& hot_swap = RecorderConfig::Instance()
            .getAttribPairFromTag("HotSwap", "timeBased");
    const MPair<QString, QString>& max_size = RecorderConfig::Instance()
            .getAttribPairFromTag("HotSwap", "maxSize");
    const MPair<QString, QString>& interval = RecorderConfig::Instance()
            .getAttribPairFromTag("HotSwap", "interval");


    if (hot_swap.m_type1 != "") {
        if (hot_swap.m_type2 == "enabled" ||
                hot_swap.m_type2 == "true") {
            // setup timer based
            Logger::Instance().logMessage(THIS_FILE, "HotSwap is set to time based!\n");
            ulong time = 60000; // 1 min minumum
            ulong time_modifier = 1;
            if (interval.m_type1 != "") {
                bool res = false;
                const char* t_string = is_digit(interval.m_type2.toStdString().data());
                time_modifier = QString(t_string).toInt(&res);
                if (!res) {
                    time_modifier = 1; // 1 min
                }
                time = time * time_modifier;
                snprintf(init_msg, sizeof(init_msg),"Time interval is: (%ld)\n", time);
                Logger::Instance().logMessage(THIS_FILE, init_msg);
            }
            // set the timer
            m_hotswap.setInterval(time);
            connect(&m_hotswap, SIGNAL(timeout()), this, SLOT(hotSwapFiles()));
            m_hotswap.start();
        } else {
            // setup filesize change
            Logger::Instance().logMessage(THIS_FILE, "HotSwap is set to file size changed!\n");
            if (max_size.m_type1 != "") {
                bool res = false;
                ulong max_size_modifier = 1;
                if (max_size.m_type2.contains("MB", Qt::CaseInsensitive) ||
                        max_size.m_type2.contains("M", Qt::CaseInsensitive)) {
                    max_size_modifier = 1000000;
                } else if (max_size.m_type2.contains("GB", Qt::CaseInsensitive) ||
                           max_size.m_type2.contains("G", Qt::CaseInsensitive)) {
                    max_size_modifier = 1000000000;
                }
                const char* size = is_digit(max_size.m_type2.toStdString().data());
                ulong mfs = QString(size).toLong(&res);
                m_maxFileSize = mfs * max_size_modifier;

                if(!res) {
                    m_maxFileSize = 30000000; // 30Mb
                }
            }
            snprintf(init_msg, sizeof(init_msg), "File size limit is: (%d) bytes\n", m_maxFileSize);
            Logger::Instance().logMessage(THIS_FILE, init_msg);

            connect(&m_filewatcher, SIGNAL(fileChanged(QString)),
                    this, SLOT(performHotSwap(QString)));
        }
    } else {
        // setup the default logic
        // swap by size
        connect(&m_filewatcher, SIGNAL(fileChanged(QString)),
                this, SLOT(performHotSwap(QString)));
    }

    // init json writter
    m_jsonWritter.init();

    return res;
}

void Recorder::deinit()
{
    Logger::Instance().logMessage(THIS_FILE, "Deinitializing recorder...\n");
    Logger::Instance().logMessage(THIS_FILE, "Closing all opened records...\n");
    for(int i=0; i < m_maxChans; ++i) {
        if (m_wavs[i] != nullptr && m_wavs[i]->isOpened()) {
            static char msg[256] = {0};
            snprintf(msg, sizeof(msg), "Closing file: (%s)\n", m_wavs[i]->getFileName());
            Logger::Instance().logMessage(THIS_FILE, msg);
            m_wavs[i]->close();
            delete m_wavs[i];
            m_wavs[i] = nullptr;
        }
    }

    m_jsonWritter.deinit();
}

WavIface *Recorder::getWavByName(const QString &fname)
{
    for(int i=0; i < m_maxChans; ++i) {
        // fixed a bug with calling compare() here isntead of
        // QString == QString
        if (fname == (QString(m_wavs[i]->getFileName()))) {
            return m_wavs[i];
        }
    }
    return nullptr;
}

/// setup all wav files for writing
/// \brief Recorder::setupWavFiles
/// \return true bu default
bool Recorder::setupWavFiles()
{
    bool res = true;

    int samples_per_sec = 0;
    int bits_per_sec = 0;
    int riff_len = 0;
    int fmt_len = 0;
    short audio_fmt = 0;
    short chann_cnt = 0;

    // careful now: we dont want the parser to fail and to
    // generate bad stuff to our wave header, the
    // parseInt() and parseShort() can return bool for success
    // if we fail it, then we assign our custom default wav header
    // valuse
    PairList& attribs = RecorderConfig::Instance().getTagPairs("Wave");

    for(int i=0; i < attribs.count(); ++i) {
        bool parse_result = false; // careful when converting to int
        MPair<QString, QString> it = attribs.at(i);
        if (it.m_type1 == "samplesPerFrame") {
            samples_per_sec = it[it.m_type1].toInt(&parse_result);
            // failsafe
            if (!parse_result) {
                samples_per_sec = 8000;
            }
        } else if (it.m_type1 == "bitsPerSec") {
            bits_per_sec = it[it.m_type1].toInt(&parse_result);
            if (!parse_result) {
                bits_per_sec = 16;
            }
        } else if (it.m_type1 == "fmtLength") {
            fmt_len = it[it.m_type1].toInt(&parse_result);
            if (!parse_result) {
                fmt_len = 16;
            }
        } else if (it.m_type1 == "audioFormat") {
            audio_fmt = it[it.m_type1].toShort(&parse_result);
            if (!parse_result) {
                audio_fmt = 1;
            }
        } else if (it.m_type1 == "channels") {
            chann_cnt = it[it.m_type1].toShort(&parse_result);
            if (!parse_result) {
                chann_cnt = 1;
            }
        } else if (it.m_type1 == "endiness"){
            // if we require endian swap for the data
        } else {
            // misra else - unused
        }
    }

    // maxfile size is set from init...

    // one time setup all waves at a time
    // don`t open them yet... do this later, after server
    // init and binding
    for(int i=0; i < m_maxChans; ++i) {
        m_wavs[i]->setupWave(samples_per_sec, bits_per_sec, riff_len,
                             fmt_len, audio_fmt, chann_cnt);
    }
    m_wavParams.samples_per_sec = samples_per_sec;
    m_wavParams.bits_per_sec = bits_per_sec;
    m_wavParams.riff_len = riff_len;
    m_wavParams.fmt_len = fmt_len;
    m_wavParams.audio_fmt = audio_fmt;
    m_wavParams.chann_cnt = chann_cnt;

    return res;
}

/// UNUSED!!!
/// buffered recording for future use
/// and for now for use of writing error packets
/// \brief Recorder::record
/// \param buffered packets write a buffer of packets
///
void Recorder::record(QQueue<udp_data_t> &packets)
{
    while (!packets.isEmpty()) {
        for(int j=0; j < m_maxChans; ++j) {
            if (m_wavs[j] != nullptr && m_wavs[j]->isOpened()) {
                // TODO: test!
                // test plugin iface
                const udp_data_t& data = packets.dequeue();
                m_wavs[j]->write((short*) data.data[j], 16);
            }
        }
    }
}

/// asume the file is opened and setup,
/// now we handle the signals from the server
/// and write the sample data to a specific slot
/// \brief Recorder::record
/// \param data - samples
///
void Recorder::record(const udp_data_t &data)
{
    // flip the data for dimo`s recorder wich comes 16 x 32
    // unlike the other device that is 32 x 16, for me
    // it`s more friendly to write one time 16 samples so
    // I`ll flip  ROW x COL and COL x ROW to be fwrite friendly
    // this is a macrodef for tests: in the final requirements
    // this structure is still unknown but the idea will be similar.

#if (REQUIRE_FLIP_CHANNS_SAMPLES)
    int16_t flip_data[32][16];

    for(int i=0; i < 32; ++i) {
        for(int j=0; j < 16; ++j) {
            flip_data[i][j] = data.data[j][i];
        }
    }
#endif
    for(int i=0; i < m_maxChans; ++i) {
        if (m_wavs[i] != nullptr && m_wavs[i]->isOpened()) {
            // TODO: test!

            const QList<RecIface>& plugins = SApplication::m_plugins;

            // pass the data to multiple plugins
            // TODO: fill in later when a specific need is pending.
            if (/*plugins.count() > */0) {
                for(int j=0; j < plugins.count(); ++j) {
                    // get the next plugin
                    const RecIface& next_plugin = plugins.at(j);

                    // make a copy of the data
                    short* copy_data = new short[16];

                    // pass it to the filter
                    next_plugin.put_ndata((short*) copy_data, 16);
                    // write to wave file and write a meta info too
                }
            } else {
// if flip is required - for now as macrodef
// later I'll cleanup the whole thing after
// a fixed structure is decided.
#if (REQUIRE_FLIP_CHANNS_SAMPLES)
            m_wavs[i]->write((short*) flip_data[i], 16);
            static char json[512]  = {0};
            snprintf(json, sizeof(json),
            "{'%s' : ["
            "{'%s' : '%d'}"
            "]}\n", m_wavs[i]->getFileName(),
                     "slot", m_wavs[i]->getSlot());
            m_jsonWritter.add(json).write();
#else
                // noplugins - I will rewrite the logic later
                m_wavs[i]->write((short*) data.data[i], 16);
#endif
            }
        }
    }
}

// unused for now
void Recorder::record(const tcp_data_t &data)
{
    (void) data;
    Logger::Instance().logMessage(THIS_FILE, "STUB! Record from TCP!\n");
}

/// Timer based hotswap, if time elapses
/// we swap files
/// \brief Recorder::hotSwapFiles
/// hotswaps wave files with new ones when
/// a given recorder time elapses
void Recorder::hotSwapFiles()
{
    for(int i=0; i < m_maxChans; ++i) {
        if (m_wavs[i] != nullptr && m_wavs[i]->isOpened()) {
            // кой писал писал
            if (1) {
                s_UID++;
                m_filewatcher.removePath(m_wavs[i]->getFileName());
                static char buff[256] = {0};
                if (m_directory != "") {
                    snprintf(buff, sizeof(buff), "%s/%d-%d-%s.wav",
                            m_directory.toStdString().data(),
                            i, s_UID, DateTime::getTimeString());

                } else {
                    snprintf(buff, sizeof(buff), "%d-%d-%s.wav",
                                i,
                                s_UID, DateTime::getTimeString());

                }
                m_wavs[i]->close();
                // TODO: rename after a name pattern is set
                //m_wavs[i]->renameFile(m_wavs[i]->getFileName(), buff);

                delete m_wavs[i];
                m_wavs[i] = nullptr;

                // open a new file in the same slot
                m_wavs[i] = new Wav(buff);
                m_wavs[i]->setupWave(m_wavParams.samples_per_sec,
                                     m_wavParams.bits_per_sec,
                                     m_wavParams.riff_len,
                                     m_wavParams.fmt_len,
                                     m_wavParams.audio_fmt,
                                     m_wavParams.chann_cnt);
                m_wavs[i]->open(i);
                m_filewatcher.addPath(m_wavs[i]->getFileName());
            }
        }
    }
}

/// \brief Recorder::performHotSwap
/// \param file - name of file
///
void Recorder::performHotSwap(const QString &file)
{
    Wav* w = (Wav*)getWavByName(file);
    if (w != nullptr && w->isOpened()) {
        if (w->getFileSize() > m_maxFileSize) {
            m_filewatcher.removePath(w->getFileName());
            s_UID++;
            int slot = w->getSlot();
            m_wavs[slot]->close();
            delete m_wavs[slot];
            m_wavs[slot] = nullptr;
            static char buff[256] = {0};
            if (m_directory != "") {
                snprintf(buff, sizeof(buff), "%s/%d-%d-%s.wav",
                        m_directory.toStdString().data(),
                        slot, s_UID, DateTime::getTimeString());
            } else {
                snprintf(buff, sizeof(buff), "%d-%d-%s.wav",
                            slot,
                            s_UID, DateTime::getTimeString());
            }
            m_wavs[slot] = new Wav(buff);
            m_wavs[slot]->setupWave(m_wavParams.samples_per_sec,
                                    m_wavParams.bits_per_sec,
                                    m_wavParams.riff_len,
                                    m_wavParams.fmt_len,
                                    m_wavParams.audio_fmt,
                                    m_wavParams.chann_cnt);
            m_wavs[slot]->open(slot);
            m_filewatcher.addPath(m_wavs[slot]->getFileName());
        }
    }
}

} // iz
