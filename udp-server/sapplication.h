#ifndef SAPPLICATION_H
#define SAPPLICATION_H

// parent //
#include <QCoreApplication>

// application stuff //
#include "plugin-manager.h"
#include "recorder.h"
#include "server.h"
#include "utils/wav-writer.h"
#include "utils/recorder-config.h"

namespace iz {

// application will connect
// server and recorder via
// signals and slots
class SApplication : public QCoreApplication
{
    Q_OBJECT
public:

    static void writeToSocket(const char* data);

    explicit SApplication(int& argc, char** argv);
    virtual ~SApplication();
    int init();
    void deinit();

public:
    // new idea: will be able to handle all ...
    // the socket concpet for the signal handlers
    static int m_fdHUP;
    static int m_fdTERM;

    static int s_argc;
    static char** s_argv;

private:
    void loadPlugins();
    void initAllPlugins();
    void proxyMainAll(int argc, char** argv);

private:
    Server              m_server;
    Recorder            m_recorder;
    QString             m_recConf;
    bool                m_setup;
    ServerThread        m_user_server;

    static QList<RecIface>     m_plugins;
    friend class Recorder;
    friend class Server;
};

} // iz

#endif // SAPPLICATION_H
