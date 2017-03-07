#include "qcapdev.h"

#include <QAudioEncoderSettings>
#include <QAudioBuffer>
#include <QAudioProbe>
#include <QAudioRecorder>
#include <QAudioFormat>
#include <QAudioDeviceInfo>
#include <QtCore>
#include <QUrl>

// remove it later //
#include <iostream>
#include <istream>

#include <stdio.h>

// message server //
#include "ipc-msg.h"
#include "utils.h"


static const char* THIS_FILE = "qcapdev.cpp";

namespace plugin {
    namespace qrec {
    using namespace utils;

    QCapDevice* QCapDevice::s_inst = nullptr;

    QCapDevice::QCapDevice(QObject *parent)
        : QObject(parent),
          p_rec(nullptr),
          io_handle(nullptr),
          iface({0, 0, 0,
                0, 0, 0,
                0, 0, 0,
                0, {0}, 0})
    {
    }

    QCapDevice::~QCapDevice()
    {
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
        QTimer::singleShot(0, r, SLOT(hEvLoop()));
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
        if (data == nullptr) {
            return 1;
        }
        QCapDevice* r = &Instance();
        if (r->iface.nextPlugin != nullptr) {
            r->iface.nextPlugin->put_data(data);
        } else {
            QList<utils::sample_data_t>* ls = (QList<utils::sample_data_t>*)data;
            ls->clear();
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
                if (strcmp(argv[i], "-l") ==  0 ||
                           strcmp(argv[i], "--list-dev") == 0) {
                    Instance().listDevices();
                    std::cout << "exitinig...\n";
                    exit(0);
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

    void QCapDevice::listDevices()
    {
    }

    void QCapDevice::start()
    {

    }

    void QCapDevice::stop()
    {

    }

    void QCapDevice::hEvLoop()
    {

        if (1) {
            bool parse_res = false;
            int srate, ccnt, brate;
            QAudioFormat format;

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

            srate = s_rate.m_type2.toInt(&parse_res);
            if (!parse_res) {
                format.setSampleRate(8000);
            } else {
                format.setSampleRate(srate);
            }

            ccnt = chans.m_type2.toInt(&parse_res);
            if (!parse_res) {
                format.setChannelCount(1);
            } else {
                format.setChannelCount(ccnt);
            }

            format.setCodec("audio/pcm");
            format.setSampleSize(16);
            format.setByteOrder(QAudioFormat::LittleEndian);
            format.setSampleType(QAudioFormat::UnSignedInt);

            QAudioDeviceInfo info = QAudioDeviceInfo::defaultInputDevice();
            if (!info.isFormatSupported(format)) {
                utils::IPC::Instance().sendMessage(THIS_FILE, "default format not supported"
                                                   ", trying to use nearest\n");
                format = info.nearestFormat(format);
            }

            p_rec = new QAudioInput(format, this);
        }

        io_handle = p_rec->start();

        connect(io_handle, &QIODevice::readyRead,
                [=]()
        {
            QByteArray samples = io_handle->readAll();
            QList<utils::sample_data_t> ls;
            utils::sample_data_t sdata = {{0},0, 0};
            sdata.samples = (short*) samples.data();
            sdata.size = (uint32_t) samples.size();
            ls.append(sdata);
            iface.put_data((QList<utils::sample_data_t>*)&ls);
        });
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

