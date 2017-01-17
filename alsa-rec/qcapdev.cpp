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

    QCapDevice* QCapDevice::s_inst = nullptr;

    QCapDevice::QCapDevice(QObject *parent)
        : QObject(parent),
          p_rec(nullptr),
          p_probe(nullptr)
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
        connect(p_probe, &QAudioProbe::audioBufferProbed,
                   [=](const QAudioBuffer& buff)
        {
            // not working - sends bad data ...
            QList<sample_data_t> ls;
            sample_data_t sdata;
            sdata.samples = new short[buff.byteCount()];
            sdata.size = buff.byteCount();
            memcpy(sdata.samples, buff.constData(), buff.byteCount());
            ls.append(sdata);
            iface.put_data((QList<sample_data_t>*)&ls);
        });

        p_rec->setAudioInput("alsa:default");
        p_probe->setSource(p_rec);

        // lazy... do a slot later ... just testing it fast
        if (1) {
            QAudioEncoderSettings settings;
            // get it from config file
            settings.setCodec("audio/PCM");
            settings.setChannelCount(1);
            settings.setSampleRate(8000);
            settings.setBitRate(16);
            settings.setQuality(QMultimedia::HighQuality);

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

