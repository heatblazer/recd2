#include "recorder.h"

// remove later
#include <iostream>

// qt //
#include <QDir>

// utils //
#include "unix/date-time.h"
#include "utils/logger.h"
#include "utils/wav-writer.h"

static const char* THIS_FILE = "recorder.cpp";

// helpers
///////////////////////////////////////////////////////////////////////////////////
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
///////////////////////////////////////////////////////////////////////////////////

namespace plugin {
namespace rec {

// statics
uint32_t Recorder::s_UID = 0;
Recorder* Recorder::s_inst = nullptr;
struct interface_t Recorder::iface = {0,0,0,
                                      0,0,0,
                                      0,0,0};
//////////////////////////////////////////////////////////////////////////////////

Recorder::Recorder(QThread *parent)
    : QThread(parent),
      m_maxChans(0),
      m_maxFileSize(0)
{
    for(int i=0; i < 128; ++i) {
        m_wavs[i] = nullptr;
    }
    m_thread.running = false;
}

// handle with care the opened files
Recorder::~Recorder()
{
}

Recorder &Recorder::Instance()
{
    if (s_inst == nullptr) {
        s_inst = new Recorder;
    }

    return *s_inst;
}

void Recorder::init()
{
    Recorder* r = &Instance();
    char init_msg[256] = {0};
    char buff[256]={0};
    bool res = true;

    // hardcoded for now
    RecorderConfig::Instance().fastLoadFile("recorder-config.xml");


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
        r->m_maxChans = 32;
    } else {
        bool res = false;
        r->m_maxChans = max.m_type2.toInt(&res);
        if (!res || r->m_maxChans > 127 || r->m_maxChans <= 0) {
            // precatuions !!!
            r->m_maxChans = 32;
        }
    }

    for(int i=0; i < r->m_maxChans; ++i) {
        s_UID++;
        if (dir.m_type1 != "") {
            snprintf(buff, sizeof(buff), "%s/%d-%d-%s.wav",
                    dir.m_type2.toStdString().data(),
                    i,
                    s_UID,
                    DateTime::getTimeString());
            r->m_directory = dir.m_type2;
        } else {
            snprintf(buff, sizeof(buff), "%d-%d-%s.wav", i,
                    s_UID,
                    DateTime::getTimeString());
            r->m_directory.clear();
        }
        r->m_wavs[i] = new Wav(buff);
    }
    // open files when everything is ok and setup
    res &= r->setupWavFiles();

    for(int i=0; i < r->m_maxChans; ++i) {
        res &= r->m_wavs[i]->open(i);
        if (res) {
            r->m_filewatcher.addPath(r->m_wavs[i]->getFileName());
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
            r->m_hotswap.setInterval(time);
            connect(&r->m_hotswap, SIGNAL(timeout()), &Instance(), SLOT(hotSwapFiles()));
            r->m_hotswap.start();
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
                r->m_maxFileSize = mfs * max_size_modifier;

                if(!res) {
                    r->m_maxFileSize = 30000000; // 30Mb
                }
            }
            snprintf(init_msg, sizeof(init_msg),
                     "File size limit is: (%d) bytes\n", r->m_maxFileSize);
            Logger::Instance().logMessage(THIS_FILE, init_msg);

            connect(&r->m_filewatcher, SIGNAL(fileChanged(QString)),
                    &Instance(), SLOT(performHotSwap(QString)));
        }
    } else {
        // setup the default logic
        // swap by size
        connect(&r->m_filewatcher, SIGNAL(fileChanged(QString)),
                &Instance(), SLOT(performHotSwap(QString)));
    }

    Instance().setObjectName("recorder-thread");
    Instance().startRecorder();
}

void Recorder::deinit()
{
    Recorder* r = &Instance();

    Logger::Instance().logMessage(THIS_FILE, "Deinitializing recorder...\n");
    Logger::Instance().logMessage(THIS_FILE, "Closing all opened records...\n");
    for(int i=0; i < r->m_maxChans; ++i) {
        if (r->m_wavs[i] != nullptr && r->m_wavs[i]->isOpened()) {
            static char msg[256] = {0};
            snprintf(msg, sizeof(msg), "Closing file: (%s)\n",
                     r->m_wavs[i]->getFileName());
            Logger::Instance().logMessage(THIS_FILE, msg);
            r->m_wavs[i]->close();
            delete r->m_wavs[i];
            r->m_wavs[i] = nullptr;
        }
    }
    r->stopRecoder();
}

void Recorder::copy(const void *src, void *dst, int len)
{
    (void) src;
    (void) dst;
    (void) len;
}

int Recorder::put_data(void *data)
{
    printf("Recorder: put data ");
    if (iface.nextPlugin != NULL) {
        puts("next loaded plugin.");
        iface.nextPlugin->put_data(data);
    } else {
        puts("no one.");
    }
    udp_data_t* udp = (udp_data_t*) data;
    Instance().record(*udp);
    return 0;
}

int Recorder::put_ndata(void *data, int len)
{
    printf("Recorder: put data to ");
    if (iface.nextPlugin != NULL) {
        puts("next loaded plugin.");
        iface.nextPlugin->put_ndata(data, len);
    } else {
        puts(" no one.");
    }
    udp_data_t* udp = (udp_data_t*) data;
    Instance().record(*udp);
}

void *Recorder::get_data()
{
    return NULL;
}

int Recorder::main_proxy(int argc, char **argv)
{
    (void) argc;
    (void) argv;
    printf("Recorder is IMPORTANT to take arguments!!!!\n"
           "Fix it!!!\n");
    return 0;
}

interface_t *Recorder::getSelf()
{
    return &iface;
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

void Recorder::run()
{
    //exec();
    QQueue<udp_data_t> dblBuff;
    do {
        usleep(100);
        m_thread.mutex.lock();
        while (!m_thread.buffer.isEmpty()) {
            dblBuff.enqueue(m_thread.buffer.dequeue());
        }
        m_thread.mutex.unlock();

        while (!dblBuff.isEmpty()) {
            udp_data_t d = dblBuff.dequeue();
            record(d);
        }
    } while (m_thread.running);

}

void Recorder::startRecorder()
{
    m_thread.running = true;
    QThread::start();
}

void Recorder::stopRecoder()
{
    m_thread.running = false;
    wait(1000);
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
    for(int i=0; i < m_maxChans; ++i) {
        if (m_wavs[i] != nullptr && m_wavs[i]->isOpened()) {
            // TODO: test!
            // pass the data to multiple plugins
            // TODO: fill in later when a specific need is pending.
            m_wavs[i]->write((short*) data.data[i], 16);
         }
    }
}

void Recorder::record(short data[], int len)
{
    for(int i=0; i < m_maxChans; ++i) {
        if (m_wavs[i] != nullptr && m_wavs[i]->isOpened()) {
            m_wavs[i]->write(data, len);
        }
    }
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

} // rec
} // plugin

const interface_t *get_interface()
{
    interface_t* pif = plugin::rec::Recorder::Instance().getSelf();
    pif->init = &plugin::rec::Recorder::init;
    pif->deinit = &plugin::rec::Recorder::deinit;
    pif->put_data = &plugin::rec::Recorder::put_data;
    pif->put_ndata = &plugin::rec::Recorder::put_ndata;
    pif->get_data = &plugin::rec::Recorder::get_data;
    pif->main_proxy = &plugin::rec::Recorder::main_proxy;
    pif->getSelf = &plugin::rec::Recorder::getSelf;
    pif->copy = &plugin::rec::Recorder::copy;
    pif->none = nullptr;
    pif->nextPlugin = nullptr;

    return plugin::rec::Recorder::Instance().getSelf();
}


///////////////////////////////////////////////////////////////////////////////////
