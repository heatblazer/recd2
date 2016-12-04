#include "sapplication.h"
#include "defs.h"

#include <iostream> // remove it later

// qt stuff//
#include <QDir>

// local headers //
#include "utils/logger.h"
#include "unix/daemon.h"

static const char* THIS_FILE = "sapplication.cpp";

namespace iz {

int SApplication::m_fdHUP = -1;
int SApplication::m_fdTERM = -1;
int SApplication::s_argc = 0;
char** SApplication::s_argv = 0;
QList<RecIface> SApplication::m_plugins;

/// TODO: write to the file desc of the app
/// \brief SApplication::writeToSocket
/// \param data
///
void SApplication::writeToSocket(const char *data)
{
    (void) data;
}

/// the inheritee of QApplication is
/// responsible for initing all stuff
/// \brief SApplication::SApplication
/// \param argc
/// \param argv
///
SApplication::SApplication(int &argc, char **argv)
    : QCoreApplication(argc, argv),
      m_setup(false)
{
    // proxy them to the plugins if needed
    s_argc = argc;
    s_argv = argv;

    // just program name
    if (argc == 1) {
        // initializing with default setting
        std::cout << "Warning! You are not using a config file! Loading defaults!"
                  << std::endl;
        m_setup = RecorderConfig::Instance().loadDefaults();
    } else {
        // wakt the args
        for(int i=0; i < argc; ++i) {
            if (strcmp(argv[i], "-c") == 0 ||
                strcmp(argv[i], "--config") == 0) {
                if (argv[i+1] != nullptr) {
                    // init config file

                  // I will test the fast loading of XML file here
#ifndef UNSAFE_CONFIG
                    // this will be safe check element by element and attribs
                    // and will return true or false
                    m_setup = RecorderConfig::Instance().loadFile(QString(argv[i+1]));
#else
                    // this is lab tested xml parsing assuming
                    // config is OK, no checks and always return TRUE
                    // which is dangerous in release
                    m_setup = RecorderConfig::Instance().fastLoadFile(QString(argv[i+1]));
                    if (!m_setup) {
                        std::cout << "Warning! You did not point a config file after: ("
                                  << argv[i] << ") argument! Loading application with defaults!"
                                  << std::endl;
                        m_setup = RecorderConfig::Instance().loadDefaults();
                    }
#endif
                }
            } else if (strcmp(argv[i], "-d") == 0 ||
                       strcmp(argv[i], "--daemon") == 0) {
                Logger::Instance().logMessage(THIS_FILE,
                                              "Program is running in daemon mode!\n");
                m_setup = RecorderConfig::Instance().loadDefaults();
            } else {
                // it`s unknown stuff to me...
                // should never fall here since main(...) won`t allow it
            }
        }
    }

    // load plugins to the app
    // I`ll better call that in the constructor
    // since I may want to pass some args to
    // the main proxies
    // for now nothing...
    Logger::Instance().logMessage(THIS_FILE, "Loading pluggins...\n");

    // plugin setups
    loadPlugins();
    proxyMainAll(argc, argv);


    // old version:
    // daemon registration of this app to be used in the
    // future signal handlers and do some stuff there
    // I`ve writen a note why a static global instance is needed
    // for this, since sigactions calbback`s user data does not
    // refer to a concrete stuff and can`t be casted to what
    // we need.
    // new version:
    // will use socket pair to connect damon to the sapplication
    // and interprocess comunicate between it
    if (m_setup) {
        Daemon::registerAppData(this);
    }
}

SApplication::~SApplication()
{
    // if something bad happens, the dtor won`t be called
    // in daemon logic we have a handler to do the deinit procedure
}

/// init all modules that we`ll need here
/// \brief SApplication::init
/// \return -1 if something is not OK in the initialization
///
int SApplication::init()
{
    bool log_init = Logger::Instance().init();
    if (!log_init) {
        return -1;
    }

    // important ! inti all plugins before
    initAllPlugins();

    Logger::Instance().logMessage(THIS_FILE, "Initializing application...\n");
    if (!m_setup) {
        Logger::Instance().logMessage(THIS_FILE, "Failed to initialize application!\n");
        return -1;
    } else {
        // transport initialization
        bool udp = false;
        quint16 port = 0;
        const MPair<QString, QString>& trans_attr =
                RecorderConfig::Instance().getAttribPairFromTag("Network", "transport");

        const MPair<QString, QString>& port_attr = RecorderConfig::Instance()
                .getAttribPairFromTag("Network", "port");

        bool parese_res = false;
        port = port_attr.m_type2.toInt(&parese_res);
        if (!parese_res) {
            char msg[128] = {0};
            snprintf(msg, sizeof(msg), "Parsing (%s) failed. Will use default: (%d)\n",
                    port_attr.m_type2.toStdString().data(),
                    1234);
            Logger::Instance().logMessage(THIS_FILE, msg);
            port = 1234U;
        }

        if (trans_attr.m_type2 == "udp") {
            udp = true;
        } else {
            udp = false;
        }

        // non plugin version
#if 0
        // they need not to depend each other
        if (!m_recorder.init()) {
            Logger::Instance().logMessage(THIS_FILE, "Failed initialize recorder\n");
            return -1;
        }
#endif

#ifdef HEARTATTACK
        bool attack = true;
#else
        bool attack = false;
#endif

        // non plugin version
#if 0
        // this is for test purpose only! Remove later!!!
        m_server.init(udp, port, attack);

        // connect rec to server
        if (udp) {
            connect(&m_server, SIGNAL(dataReady(udp_data_t)),
                &m_recorder, SLOT(record(udp_data_t)));

            connect(&m_server, SIGNAL(dataReady(QQueue<udp_data_t>&)),
                    &m_recorder, SLOT(record(QQueue<udp_data_t>&)));

        } else {
            connect(&m_server, SIGNAL(dataReady(tcp_data_t)),
                    &m_recorder, SLOT(record(tcp_data_t)));
        }
#endif
    }

    Logger::Instance().logMessage(THIS_FILE, "Initialization of application completed!\n");

    // register user server
    m_user_server.setObjectName("user server");
    m_user_server.moveToThread(&m_user_server);
    m_user_server.start();

    return 0;

}

/// stop all stuff
void SApplication::deinit()
{
    Daemon::log("SApplication::deinit()!\n");

    // close all files that are being recorded
    //m_recorder.deinit();

    // stop the server ... if something has to be done
    //m_server.deinit();

    // finally deinit plugins, since somebody may still
    // depend on them... better make sure your plugin
    // has finished it`s job to prevent artifacts
    interface_t* it = RecPluginMngr::getPluginList().getFront();
    while (it != nullptr) {
        it->deinit();
        it = it->nextPlugin;
    }

#if 0
    for(int i=0; i < m_plugins.count(); ++i) {
        // deinit in priority order
        m_plugins.at(i).deinit();
    }
#endif

    m_user_server.wait(1000);
}

void SApplication::loadPlugins()
{
    // plugin setup section
    // this is a bit toug logic for now
    PairList list = RecorderConfig::Instance().getTagPairs("Plugin");
    char msg[256] = {0};

    for(int i=0; i < list.count(); ++i) {
       if (list.at(i).m_type1 == "name" && list.at(i).m_type2 != "") {
            // perform the parsina and plugin setup here
            // the array is ordered and we assume name is
            // in the front
           snprintf(msg, sizeof(msg), "Loading (%s) plugin...\n",
                   list.at(i).m_type2.toStdString().data());
           Logger::Instance().logMessage(THIS_FILE, msg);
           // defensive programming - array out of boudns prevention!
           const RecIface* iface = nullptr;

           if (i+3 > list.count()) {
               // don`t load item since program will crash!!!
           } else {
               ASSERT_MACRO(list.at(i+3).m_type2.toStdString().data() != NULL);
               RecPluginMngr::loadLibrary(list.at(i+3).m_type2, list.at(i).m_type2);
               iface = RecPluginMngr::getInterface(list.at(i).m_type2);
           }

           // put in any order for now
           // store into the indexed array
           if (iface != nullptr) {
               snprintf(msg, sizeof(msg), "Loaded (%s) plugin.\n", list.at(i).m_type2.toStdString().data());
               Logger::Instance().logMessage(THIS_FILE, msg);
               m_plugins.push_back(*iface);
           } else {
               snprintf(msg, sizeof(msg), "\nFailed to load (%s) plugin.\n",
                       list.at(i).m_type2.toStdString().data());
                Logger::Instance().logMessage(THIS_FILE, msg);
           }
        }
    }
}

/// init and prepare all plugins, assume the plugin
/// provider has implemented init(...)
/// \brief SApplication::initAllPlugins
///
void SApplication::initAllPlugins()
{
    interface_t* it = RecPluginMngr::getPluginList().getFront();
    while (it != nullptr) {
        it->init();
        it = it->nextPlugin;
    }
}

void SApplication::proxyMainAll(int argc, char **argv)
{
    interface_t* it = RecPluginMngr::getPluginList().getFront();
    while (it != nullptr) {
        it->main_proxy(argc, argv);
        it = it->nextPlugin;
    }
}


} // iz
