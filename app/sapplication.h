#ifndef SAPPLICATION_H
#define SAPPLICATION_H

#include <QCoreApplication>

#include "utils.h"
#include "plugin-manager.h"
#include "server.h"

namespace recd {

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
private slots:
    void hEvLoop();
private:
    void loadPlugins();
    void initAllPlugins();
    void proxyMainAll(int argc, char** argv);

private:

    QString             m_recConf;
    bool                m_setup;
    ServerThread        m_user_server;
};

} // recd

#endif // SAPPLICATION_H
