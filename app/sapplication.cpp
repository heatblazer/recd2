#include "sapplication.h"

#include <QDir>
#include <iostream> // remove it later

#include "defs.h"
#include "utils.h"
#include "unix/daemon.h"

static const char* THIS_FILE = "sapplication.cpp";

namespace recd {

int SApplication::m_fdHUP = -1;
int SApplication::m_fdTERM = -1;
int SApplication::s_argc = 0;
char** SApplication::s_argv = 0;

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
        m_setup = utils::RecorderConfig::Instance().loadDefaults();
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
                    m_setup = utils::RecorderConfig::Instance().fastLoadFile(QString(argv[i+1]));
                    if (!m_setup) {
                        std::cout << "Warning! You did not point a config file after: ("
                                  << argv[i] << ") argument! Loading application with defaults!"
                                  << std::endl;
                        m_setup = utils::RecorderConfig::Instance().loadDefaults();
                    }
#endif
                }
            } else if (strcmp(argv[i], "-d") == 0 ||
                       strcmp(argv[i], "--daemon") == 0) {
                m_setup = utils::RecorderConfig::Instance().loadDefaults();
            } else {
                // it`s unknown stuff to me...
                // should never fall here since main(...) won`t allow it
            }
        }
    }

    // register user server and message server
    // make sure it is registered so all messages from
    // plugins start flowing trough it.
    m_user_server.setObjectName("user server");
    m_user_server.moveToThread(&m_user_server);
    m_user_server.start();

    // plugin setups
    loadPlugins();
    // proxy to main, pass dependent stuff here
    // ex. Recorder needs to know the config.xml where it was passed
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

    bool log_init = utils::Logger::Instance().init();
    if (!log_init) {
        return -1;
    }

    // important ! inti all plugins before
    initAllPlugins();

    utils::Logger::Instance().logMessage(THIS_FILE, "Initializing application...\n");
    if (!m_setup) {
        utils::Logger::Instance().logMessage(THIS_FILE, "Failed to initialize application!\n");
        return -1;
    } else {
        // transport initialization
        bool udp = false;
        (void) udp;
        quint16 port = 0;
        (void) port;
        const utils::MPair<QString, QString>& trans_attr =
                utils::RecorderConfig::Instance().getAttribPairFromTag("Network", "transport");

        const utils::MPair<QString, QString>& port_attr = utils::RecorderConfig::Instance()
                .getAttribPairFromTag("Network", "port");

        bool parese_res = false;
        port = port_attr.m_type2.toInt(&parese_res);
        if (!parese_res) {
            char msg[128] = {0};
            snprintf(msg, sizeof(msg), "Parsing (%s) failed. Will use default: (%d)\n",
                    port_attr.m_type2.toStdString().data(),
                    1234);
            utils::Logger::Instance().logMessage(THIS_FILE, msg);
            port = 1234U;
        }

        if (trans_attr.m_type2 == "udp") {
            udp = true;
        } else {
            udp = false;
        }
    }
    utils::Logger::Instance().logMessage(THIS_FILE, "Initialization of application completed!\n");
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
    utils::Logger::Instance().deinit();
    m_user_server.wait(1000);
}

void SApplication::hEvLoop()
{
    // important ! inti all plugins before
}

void SApplication::loadPlugins()
{
    // plugin setup section
    // this is a bit toug logic for now
    utils::PairList list = utils::RecorderConfig::Instance().getTagPairs("Plugin");
    char msg[256] = {0};

    for(int i=0; i < list.count(); ++i) {
       if (list.at(i).m_type1 == "name" && list.at(i).m_type2 != "") {
            // perform the parsina and plugin setup here
            // the array is ordered and we assume name is
            // in the front
           snprintf(msg, sizeof(msg), "Loading (%s) plugin...\n",
                   list.at(i).m_type2.toStdString().data());
           utils::Logger::Instance().logMessage(THIS_FILE, msg);
           if (i+3 > list.count()) {
               // don`t load item since program will crash!!!
           } else {
               ASSERT_MACRO(list.at(i+3).m_type2.toStdString().data() != NULL);
               bool res = false;
               res = RecPluginMngr::loadLibrary(list.at(i+3).m_type2, list.at(i).m_type2);
               const char* plugin = list.at(i).m_type2.toStdString().data();
               if (!res) {
                   snprintf(msg, sizeof(msg), "Failed to load: (%s) plugin.\n",
                            plugin);
                   utils::Logger::Instance().logMessage(THIS_FILE, msg);
               } else {
                   snprintf(msg, sizeof(msg), "Loaded: (%s) plugin.\n",
                            plugin);
                   utils::Logger::Instance().logMessage(THIS_FILE, msg);
               }
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
    if (it == nullptr) {
        return;
    }
    while (it != nullptr) {
        it->init();
        it = it->nextPlugin;
    }
    m_setup = true;
}

void SApplication::proxyMainAll(int argc, char **argv)
{
    interface_t* it = RecPluginMngr::getPluginList().getFront();
    while (it != nullptr) {
        it->main_proxy(argc, argv);
        it = it->nextPlugin;
    }
}

} // recd
