#include "qcapdev.h"

#include <QAudioEncoderSettings>
#include <QAudioBuffer>
#include <QAudioProbe>
#include <QAudioRecorder>
#include <QtCore>
#include <QUrl>

// remove it later //
#include <iostream>
#include <istream>

// message server //
#include "ipc-msg.h"
#include "utils.h"

namespace plugin {
    namespace qrec {
    using namespace utils;

    QCapDevice* QCapDevice::s_inst = nullptr;

    QCapDevice::QCapDevice(QObject *parent)
        : QObject(parent),
          p_rec(nullptr),
          p_probe(nullptr),
          iface({0, 0, 0,
                0, 0, 0,
                0, 0, 0,
                0, {0}, 0})
    {
        p_rec = new QAudioRecorder;
        p_probe = new QAudioProbe;

    }

    QCapDevice::~QCapDevice()
    {
        if (p_probe != nullptr) {
            delete p_probe;
            p_probe = nullptr;
        }
    }

    QCapDevice &QCapDevice::Instance()
    {
        if (s_inst == nullptr) {
            s_inst = new QCapDevice ;
        }
        return *s_inst;
    }

    void QCapDevice::init()
    {
        QCapDevice*r = &Instance();
        QTimer::singleShot(1000, r, SLOT(hEvLoop()));
    }

    void QCapDevice::deinit()
    {
    }

    void QCapDevice::copy(const void *src, void *dst, int len)
    {
        (void) src; (void) dst; (void) len;
    }

    int QCapDevice::put_data(void *data)
    {
        QCapDevice* r = &Instance();
        if (r->iface.nextPlugin != nullptr) {
            r->iface.nextPlugin->put_data(data);
        }
    }

    int QCapDevice::put_ndata(void *data, int len)
    {
        QCapDevice* r = &Instance();
        if (r->iface.nextPlugin != nullptr) {
            r->iface.nextPlugin->put_ndata(data, len);
        }
    }

    void *QCapDevice::get_data()
    {
        return nullptr;
    }

    void QCapDevice::setName(const char *name)
    {
        QCapDevice* r = &Instance();
        strncpy(r->iface.name, name, 256);
    }

    const char *QCapDevice::getName()
    {
        QCapDevice* r = &Instance();
        return r->iface.name;
    }

    int QCapDevice::main_proxy(int argc, char **argv)
    {
        if (argc < 2) {
            return -1;
        } else {
            // omit program name
            for(int i=1; i < argc; ++i) {
                if (strcmp(argv[i], "-c") == 0 ||
                    strcmp(argv[i], "--config") == 0) {
                    if (i+1 > argc) {
                        return -1;
                    } else {
                        bool res = RecorderConfig::Instance()
                                .fastLoadFile(QString(argv[i+1]));
                        if (res) {
                            // write some msg
                        }
                        RecorderConfig* r = &RecorderConfig::Instance();
                        int i = 0;
                    }
                }
            }
            return 0;
        }

    }

    interface_t *QCapDevice::getSelf()
    {
        QCapDevice* r = &Instance();
        return &r->iface;
    }

    void QCapDevice::start()
    {
        p_rec->record();
    }

    void QCapDevice::stop()
    {
        p_rec->stop();
    }

    void QCapDevice::hEvLoop()
    {
        const MPair<QString, QString>& device =
                RecorderConfig::Instance().getAttribPairFromTag("QAudioCapture", "device");

        const MPair<QString, QString>& s_rate =
                RecorderConfig::Instance().getAttribPairFromTag("QAudioCapture", "sampleRate");

        const MPair<QString, QString>& bit_rate =
                RecorderConfig::Instance().getAttribPairFromTag("QAudioCapture", "bitRate");

        const MPair<QString, QString>& chans =
                RecorderConfig::Instance().getAttribPairFromTag("QAudioCapture", "chans");

        const MPair<QString, QString>& quality =
                RecorderConfig::Instance().getAttribPairFromTag("QAudioCapture", "quality");

        p_rec->setAudioInput(device.m_type2);
        p_probe->setSource(p_rec);

        // lazy... do a slot later ... just testing it fast
        // for now use lambds
        // lazy C++11 syntax, refractor later
        connect(p_probe, &QAudioProbe::audioBufferProbed,
                   [=](const QAudioBuffer& buff)
        {
            // not working - sends bad data ...
            QList<utils::sample_data_t> ls;
            utils::sample_data_t sdata = {0, 0};
            sdata.samples = new short[buff.byteCount()];
            sdata.size = (uint32_t) buff.byteCount();
            memcpy(sdata.samples, buff.constData<short>(), buff.byteCount());
            ls.append(sdata);
            iface.put_data((QList<utils::sample_data_t>*)&ls);
        });

        if (1) {

            bool parse_res = false;
            int sample_rate = 0 , brate = 0, ccnt = 0;
            QAudioEncoderSettings settings;
            // get it from config file
            settings.setCodec("audio/PCM");

            // channels
            ccnt = chans.m_type2.toInt(&parse_res);
            if (parse_res) {
                settings.setChannelCount(ccnt);
            } else {
                settings.setChannelCount(1);
            }

            // sample rate
            sample_rate = s_rate.m_type2.toInt(&parse_res);
            if (parse_res) {
                settings.setSampleRate(sample_rate);
            } else {
                settings.setSampleRate(8000);
            }

            // bit rate
            brate = bit_rate.m_type2.toInt(&parse_res);
            if (parse_res) {
                settings.setBitRate(brate);
            } else {
                settings.setBitRate(16);
            }

            settings.setQuality(QMultimedia::NormalQuality);
            settings.setEncodingMode(QMultimedia::ConstantQualityEncoding);
            p_rec->setOutputLocation(QUrl());
            p_rec->setAudioSettings(settings);
            p_rec->setContainerFormat("raw");
        }

        p_rec->record();
    }

    } // qrec
} // plugin


////////////////////////////////////////////////////////////////////////////////
const interface_t *get_interface()
{
    interface_t* piface = plugin::qrec::QCapDevice::Instance().getSelf();

    piface->init = &plugin::qrec::QCapDevice::init;
    piface->deinit = &plugin::qrec::QCapDevice::deinit;
    piface->copy = &plugin::qrec::QCapDevice::copy;
    piface->put_data = &plugin::qrec::QCapDevice::put_data;
    piface->put_ndata = &plugin::qrec::QCapDevice::put_ndata;
    piface->get_data = &plugin::qrec::QCapDevice::get_data;
    piface->main_proxy = &plugin::qrec::QCapDevice::main_proxy;
    piface->getSelf   = &plugin::qrec::QCapDevice::getSelf;
    piface->setName = &plugin::qrec::QCapDevice::setName;
    piface->getName = &plugin::qrec::QCapDevice::getName;
    piface->nextPlugin = nullptr;

    return plugin::qrec::QCapDevice::Instance().getSelf();
}

