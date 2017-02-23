#include "recorder.h"

#include <QDir>

// remove it after tests passed //
#include <iostream>

#include "date-time.h"
#include "ipc-msg.h"
#include "wav-writer.h"

using namespace utils;

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
                                          0,0,0,
                                          0,{0},0};
////////////////////////////////////////////////////////////////////////////////

    Recorder::Recorder(QObject *parent)
        : QObject(parent),
          m_maxChans(0),
          m_sizeBased(false),
          m_maxFileSize(0)
    {
        m_thread.running = false;
        m_thread.speed = 100;
        for(int i=0; i < Chans::Count; ++i) {
            m_wavs[i] = nullptr;
        }
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

        utils::IPC::Instance().sendMessage(THIS_FILE, "Initializing recorder...\n");
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
                RecorderConfig::Instance().getAttribPairFromTag("FrameData", "channels");
        if (max.m_type1 == "") {
            r->m_maxChans = 1;
        } else {
            bool res = false;
            r->m_maxChans = max.m_type2.toInt(&res);
            if (!res || r->m_maxChans > 127 || r->m_maxChans <= 0) {
                // precatuions !!!
                r->m_maxChans = 1;
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
                IPC::Instance().sendMessage(THIS_FILE, "HotSwap is set to time based!\n");
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
                    IPC::Instance().sendMessage(THIS_FILE, init_msg);
                }
                // set the timer
                r->m_hotswap.setInterval(time);
                connect(&r->m_hotswap, SIGNAL(timeout()), r, SLOT(hotSwapFiles()));
                r->m_hotswap.start();
            } else {
                // setup filesize change
                IPC::Instance().sendMessage(THIS_FILE, "HotSwap is set to file size changed!\n");
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
                IPC::Instance().sendMessage(THIS_FILE, init_msg);
                r->m_sizeBased = true;
            }
        } else {
            // setup the default logic
            // swap by size
        }
        r->m_thread.mutex.init();
        r->m_thread.thread.setThreadName("recorder thread");
        Instance().startRecorder();
    }

    void Recorder::deinit()
    {
        Recorder* r = &Instance();
        IPC::Instance().sendMessage(THIS_FILE, "Deinitializing recorder...\n");
        IPC::Instance().sendMessage(THIS_FILE, "Closing all opened records...\n");

        for(int i=0; i < Chans::Count; ++i) {
            if (r->m_wavs[i] != nullptr) {
                if (r->m_wavs[i]->isOpened()) {
                    static char msg[256] = {0};
                    snprintf(msg, 256, "Closing file: (%s)\n",
                             r->m_wavs[i]->getFileName());
                    IPC::Instance().sendMessage(THIS_FILE, msg);
                    r->m_wavs[i]->close();
                    delete r->m_wavs[i];
                    r->m_wavs[i] = nullptr;
                }
            }
        }
        r->stopRecoder();
        r->m_thread.mutex.deinit();
        r->m_thread.thread.suspend(1000);
        r->m_thread.thread.join();
        r->m_thread.thread.closeThread();

    }

    void Recorder::copy(const void *src, void *dst, int len)
    {
        (void) src;
        (void) dst;
        (void) len;
    }

    /// takes the list of samples and polls them
    /// to the recorder thread
    /// \brief Recorder::put_data
    /// \param data
    /// \return always 0
    ///
    int Recorder::put_data(void *data)
    {
        if (data == nullptr) {
            return 1;
        }
        Recorder* r = &Instance();
        if (iface.nextPlugin != nullptr) {
            iface.nextPlugin->put_data(data);
        }

        QList<sample_data_t>* ls = (QList<sample_data_t>*)data;
        r->m_thread.mutex.lock();
        r->m_thread.buffer.enqueue(*ls);
        r->m_thread.mutex.unlock();

        // I`ve forgot to purge the list... :( Bad things can happen
        // recorder is usually last plugin so clean the list
        ls->clear();
        return 0;
    }

    /// unused
    /// \brief Recorder::put_ndata
    /// \param data
    /// \param len
    /// \return nothing
    ///
    int Recorder::put_ndata(void *data, int len)
    {
        if (iface.nextPlugin != nullptr) {
            iface.nextPlugin->put_ndata(data, len);
        }
        return 0;
    }

    /// unused
    /// \brief Recorder::get_data
    /// \return nothing
    ///
    void *Recorder::get_data()
    {
        return nullptr;
    }

    void Recorder::setName(const char *name)
    {
        strncpy(iface.name, name, 256);
    }

    const char *Recorder::getName()
    {
        return iface.name;
    }

    /// configure recorder from xml via this main proxy
    /// \brief Recorder::main_proxy
    /// \param argc
    /// \param argv
    /// \return 0 on ok -1 on fail
    ///
    int Recorder::main_proxy(int argc, char **argv)
    {
        if (argc < 2) {
            return -1;
        } else {
            for(int i=0; i < argc; ++i) {
                if ((strcmp(argv[i], "-c") == 0) ||
                    (strcmp(argv[i], "--config"))) {
                    if (argv[i+1] != nullptr) {
                        RecorderConfig::Instance().fastLoadFile(argv[i+1]);
                    }
                }
            }
        }
        return 0;
    }

    interface_t *Recorder::getSelf()
    {
        return &iface;
    }

    Wav *Recorder::getWavByName(const QString &fname)
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

    void* Recorder::run(void* pArgs)
    {
        QQueue<QList<sample_data_t> > dblBuff;
        Recorder* r = (Recorder*) pArgs;
        do {
            // setup it from outside, tweakable stuff
            //r->m_thread.thread.suspend(r->m_thread.speed);
            r->m_thread.mutex.lock();
            while (!r->m_thread.buffer.isEmpty()) {
                dblBuff.enqueue(r->m_thread.buffer.dequeue());
            }
            r->m_thread.mutex.unlock();

            for(int i=0; i < dblBuff.count(); ++i) {
                QList<sample_data_t> sd = dblBuff.at(i);
                r->record(sd);
            }

            dblBuff.clear();
            r->m_thread.thread.suspend(0);
        } while (r->m_thread.running);
    }

    /// starts recorder thread
    /// \brief Recorder::startRecorder
    ///
    void Recorder::startRecorder()
    {
        m_thread.running = true;
        //QThread::start();
        m_thread.thread.createThread(64 * 1024, 20, Recorder::run, this);
    }

    void Recorder::stopRecoder()
    {
        m_thread.running = false;
        m_thread.thread.join();
        m_thread.thread.closeThread();
    }

    /// current version of record
    /// takes a list of samples and passes them to
    /// the runner
    /// \brief Recorder::record
    /// \param list of sample data struct
    ///
    void Recorder::record(QList<sample_data_t> sd)
    {
        if (sd.count() <= 0) {
            return ;
        }
        for(int i=0; i < sd.count(); ++i) {
            if (m_sizeBased) {
                pollHotSwap();
            }
            sample_data_t s = sd.at(i);
            if (m_wavs[i] != nullptr) {
                if (m_wavs[i]->isPaused()) {
                    m_wavs[i]->close();
                } else {
                    m_wavs[i]->write(s.samples, s.size);
                }
            }
        }
        sd.clear();
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
                    char buff[256] = {0};
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
                }
            }
        }
    }

    /// check if hotswap is required
    /// it can be optimized a bit
    /// \brief Recorder::pollHotSwap
    ///
    void Recorder::pollHotSwap()
    {
        for(int i=0; i < m_maxChans; ++i) {
            if (m_wavs[i] != nullptr && m_wavs[i]->isOpened()) {
                // ако сме минали размера
                if (m_wavs[i]->getFileSize() > m_maxFileSize) {
                    s_UID++;
                    char buff[256] = {0};
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
                }
            }
        }
    }

    } // rec
} // plugin

////////////////////////////////////////////////////////////////////////////////

/// library thing
/// \brief get_interface
/// \return the current interface  to be chained in the list
///
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
    pif->setName = &plugin::rec::Recorder::setName;
    pif->getName = &plugin::rec::Recorder::getName;
    pif->nextPlugin = nullptr;
    return plugin::rec::Recorder::Instance().getSelf();
}
