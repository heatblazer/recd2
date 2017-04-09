#ifndef RECORDER_H
#define RECORDER_H

// qt stuff //
#include <QObject>
#include <QAudioInput>
#include <QAudioProbe>
#include <QFile>

#include <stdint.h>

#include "plugin-iface.h"

namespace plugin {
    namespace qrec {

    class QCapDevice : public QObject
    {
        Q_OBJECT
    public:
        static QCapDevice& Instance();
        static void init();
        static void deinit(void);
        static void copy(const void* src, void* dst, int len);
        static int put_data(void* data);
        static int put_ndata(void* data, int len);
        static void* get_data(void);
        static void setName(const char* name);
        static const char* getName(void);
        static int main_proxy(int argc, char** argv);
        static struct interface_t* getSelf(void);

        static void listDevices();
        // these require the event loop
        void start();
        void stop();

    private slots:
        void hEvLoop();
    private:
        explicit QCapDevice(QObject* parent = nullptr);
        virtual ~QCapDevice();
        static QCapDevice* s_inst;
        // maybe members later
        QAudioInput* p_rec;
        QIODevice* io_handle; // handle bytes from device
        struct interface_t iface;
    };

    } // qrec
} // plugin


#endif // RECORDER_H
