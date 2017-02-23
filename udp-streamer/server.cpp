#include "server.h"

#include <iostream>


#include <QCoreApplication>

#include "ipc-msg.h"
#include "date-time.h"
#include "recorder-config.h"
#include "types.h"

namespace plugin {
    namespace udp {

    Server* Server::s_inst = nullptr;
    interface_t Server::iface = {0,0,0,
                                 0,0,0,
                                 0,0,0,
                                 0,{0},0};
    // the err udp packet
    static struct utils::frame_data_t err_udp = {0,{0},{0}}; // warn fix
////////////////////////////////////////////////////////////////////////////////

    Server& Server::Instance()
    {
        if (s_inst == nullptr) {
            s_inst = new Server;
        }
        return *s_inst;
    }

    Server::Server(QObject *parent)
        : QObject(parent),
          udp(nullptr),
          p_server(nullptr),
          m_conn_info{0,0,0,false},
          m_channels(0),
          m_smplPerChan(0)
    {
    }

    /// simple init function
    /// \brief Server::init
    /// \param is_daemonp_server
    /// \param udp maybe and tcp later
    /// \param port
    ///
    void Server::init()
    {
        Server* s = &Server::Instance();
        printf("Initializing server...\n");
        // the error packet to be sent on packet lost

        // setup frame
        if(1) {

            // safety parse checks
            bool bres = false;

            const utils::MPair<QString, QString> chans =
                    utils::RecorderConfig::Instance()
                    .getAttribPairFromTag("FrameData", "channels");

            const utils::MPair<QString, QString> smpls =
                    utils::RecorderConfig::Instance()
                    .getAttribPairFromTag("FrameData", "samplesPerChan");

            const utils::MPair<QString, QString> msg_hdr =
                    utils::RecorderConfig::Instance()
                    .getAttribPairFromTag("FrameData", "header");

            // TODO: to be used in the future
            (void)msg_hdr;



            if (smpls.m_type1 == "") {
                s->m_smplPerChan = 16;
            } else {
                s->m_smplPerChan = smpls.m_type2.toInt(&bres);
                if (!bres) {
                    s->m_smplPerChan = 16;
                }
            }

            if (chans.m_type1 == "") {
                s->m_channels = 1;
            } else {
                s->m_channels = chans.m_type2.toInt(&bres);
                if (!bres || s->m_channels > 127 || s->m_channels <= 0) {
                    // precatuions !!!
                    s->m_channels = 1;
                }
            }

            static const int16_t max = 32111;
            for(uint32_t i=0; i < s->m_channels;) {
                for(int j=0; j < s->m_smplPerChan; ++j) {
                    err_udp.data[i++] = max;
                }
            }
        }

        // setup transport and server
        {
            bool bres = false;
            // removing constnes for a bit ...

            utils::MPair<QString, QString>* transport =
                    const_cast<utils::MPair<QString, QString>* >
                    (&utils::RecorderConfig::Instance().
                    getAttribPairFromTag("Network", "transport"));

            utils::MPair<QString, QString>* port =
                    const_cast<utils::MPair<QString, QString>* >
                    (&utils::RecorderConfig::Instance()
                    .getAttribPairFromTag("Network", "port"));


            if (port->m_type1 == "") {
                port->m_type2 = "1234";
            } else {
                s->m_port = port->m_type2.toInt(&bres);
                if (!bres) {
                    s->m_port = 1234;
                }
            }


            if (transport->m_type2 == "udp") {
                    s->udp = new QUdpSocket;
                    bres = s->udp->bind(s->m_port, QUdpSocket::ShareAddress);

                    connect(s->udp, SIGNAL(readyRead()),
                            s, SLOT(readyReadUdp())/*, Qt::DirectConnection*/);

                    if (bres) {
                        printf("Bind OK!\n");
                        s->m_liveConnection.setInterval(1000);
                        connect(&s->m_liveConnection, SIGNAL(timeout()),
                                s, SLOT(checkConnection()));
                        s->m_liveConnection.start();
                    } else {
                            printf("Bind FAIL!\n");
                            Instance().route(DISCONNECTED);
                    }
                } else if (transport->m_type2 == "tcp") {
                    // give time to everithying to init
                    QTimer::singleShot(0,
                                       [=]()
                    {
                        s->initTcpServer();
                    });

            } else {
                std::cout << "Invalid xml attribute (" << transport->m_type2.toStdString()
                          << ")" << " from Tag: ("  <<
                             transport->m_type1.toStdString() << ")" << std::endl;
                exit(1);
            }

        }

    }

    /// ready read datagrams, send to other plugins
    /// perform some packet reciever checks and other stuff
    /// \brief Server::readyReadUdp
    ///
    void Server::readyReadUdp()
    {
        static char msg[512] = {0};
        static const int SIZE = m_channels * m_smplPerChan;

        if (udp->hasPendingDatagrams()) {
            m_monitorData.append('.');

            while (udp->hasPendingDatagrams()) {

                QByteArray buff;
                buff.resize(udp->pendingDatagramSize());

                qint64 read = udp->readDatagram(buff.data(), buff.size(),
                                       &m_senderHost, &m_senderPort);
                if (read > 0) {
                    // the udp structure from the device
                    utils::frame_data_t udp = *(utils::frame_data_t*) buff.data();
                    // one frame lost for synching with my counter

                    if (udp.counter != ++m_conn_info.paketCounter) {
                        snprintf(msg, sizeof(msg),
                                 "Last synch packet:(%u)\t at: [%s]\n"
                                 "Total desynch:(%u)\n"
                                 "Server counter: (%u)\n"
                                 "Lost: (%u)\n"
                                 "Total lost: (%u)\n",
                                 udp.counter, // next got ocunter
                                 utils::DateTime::getDateTime(),       // current time
                                 m_conn_info.desynchCounter,    // desync counter
                                 m_conn_info.paketCounter,      // server counter
                                 (udp.counter - m_conn_info.paketCounter),  // lost
                                 m_conn_info.totalLost);               // total lost

                        utils::IPC::Instance().sendMessage(msg);
                        m_conn_info.desynchCounter++;
                        int errs = udp.counter - m_conn_info.paketCounter;
                        m_conn_info.totalLost += errs;
                        m_conn_info.paketCounter = udp.counter; // synch back

                        // it`s an err sender logic below
                        // always write a null bytes packet on missed udp
                        QList<utils::sample_data_t> err_ls;

                        if(!m_conn_info.onetimeSynch) {
                            m_conn_info.onetimeSynch = true;
                            for(uint32_t i=0; i < SIZE; ) {
                                utils::sample_data_t s = {0, 0};
                                short smpl[16] = {0};
                                s.samples = smpl;
                                s.size = m_smplPerChan;
                                for(uint32_t j=0; j < s.size; ++j) {
                                    s.samples[j] = err_udp.data[i++];
                                }
                                err_ls.append(s);
                            }
                            put_data((QList<utils::sample_data_t>*) &err_ls);
                        } else {
                            for(int i=0; i < errs; ++i) {
                               for(uint32_t j=0; j < SIZE; ) {
                                   utils::sample_data_t sd = {0, 0};
                                   short smpl[512] = {0};
                                   sd.samples = smpl;
                                   sd.size = m_smplPerChan;
                                   for(uint32_t h=0; h < sd.size; ++h) {
                                       sd.samples[h] = err_udp.data[j++];
                                   }
                                   err_ls.append(sd);
                               }
                               put_data((QList<utils::sample_data_t>*) &err_ls);
                            }
                        }
                    } else {
                    // will use a new logic emit the udp struct
                    // to the recorder, so now we don`t need
                    // to depend each other
                        //put_data((frame_data_t*) udp);
                        QList<utils::sample_data_t> ls;
                        // copy all the data then send it to the plugins
                        for(uint32_t i=0; i < SIZE; ) {
                            utils::sample_data_t s = {0, 0};
                            short smpl[512] = {0};
                            s.samples = smpl;
                            s.size = m_smplPerChan;
                            // fill the list to be passed to other plugins
                            for(uint32_t j=0; j < s.size; ++j) {
                                s.samples[j] = udp.data[i++];
                            }
                            ls.append(s);
                        }
                        // finally send it
                        put_data((QList<utils::sample_data_t>*) &ls);
                    }
                 } else {
                    snprintf(msg, sizeof(msg), "Missed an UDP\n");
                    utils::IPC::Instance().sendMessage(msg);
                }
            }
        } else {
            Instance().disconnected();
        }
    }


    /// check if the device is sending data
    /// send some statistics each 1 second
    /// \brief Server::checkConnection
    ///
    void Server::checkConnection()
    {
       // static char msg[512] = {0};
        if (m_monitorData.isEmpty()) {
            // not ok!
            Instance().disconnected();
            utils::IPC::Instance().sendMessage("Lost connection!\n");
        } else {
            // make sure you purge the list
            m_monitorData.clear();
        }
    }

    /// state checking router
    /// \brief Server::route
    ///
    void Server::route(States state)
    {
        // handle state in this routing function
        // handle state in this routing function
        switch (state) {
        case DISCONNECTED:
            Instance().disconnected();
            break;  // try to reconnect
        case CONNECTED:
            break;
        case LOST_CONNECTION:
        case GOT_CONNECTION:
        case GOT_DATAGRAM:
        case MISSED_DATAGRAM:
        case UNKNOWN:
        default:
            break;
        }
    }

    /// resets the counters
    /// \brief Server::disconnected
    ///
    void Server::disconnected()
    {
        m_monitorData.clear();
        m_conn_info.desynchCounter = 0;
        m_conn_info.paketCounter = 0;
        m_conn_info.totalLost = 0;
        m_conn_info.onetimeSynch = false;
    }

    void Server::initTcpServer()
    {
        p_server = new TcpServer;
        p_server->init();
    }

    /// deinitialze the server, maybe some unfinished
    /// task to be finalized here, or to be registered in
    /// the daemon, will left it a TODO
    /// \brief Server::deinit
    ///
    void Server::deinit(void)
    {
        Server* s = &Instance();
        if (s->p_server != nullptr) {
            s->p_server->deinit();
            delete s->p_server;
            s->p_server = nullptr;
        }

        utils::IPC::Instance().sendMessage("Server: deinit\n");
    }

    void Server::copy(const void *src, void *dst, int len)
    {
        (void)src; (void) dst; (void) len;
    }

    int Server::put_data(void *data)
    {
        if (data == nullptr) {
            return 1;
        }
        if (iface.nextPlugin != nullptr) {
            iface.nextPlugin->put_data(data);
        } else {
            QList<utils::sample_data_t>* ls = (QList<utils::sample_data_t>*) data;

            ls->clear();
        }
        return 0;
    }

    int Server::put_ndata(void *data, int len)
    {
        if (iface.nextPlugin != nullptr) {
            iface.nextPlugin->put_ndata(data, len);
        }
        return 0;
    }

    void *Server::get_data()
    {
        return nullptr; // dummy for now
    }

    void Server::setName(const char *name)
    {
        strncpy(iface.name, name, 256);
    }

    const char *Server::getName()
    {
        return iface.name;
    }

    int Server::p_main(int argc, char **argv)
    {

        if (argc < 2) {
            return -1;
        } else {
            for(int i=0; i < argc; ++i) {
                if ((strcmp(argv[i], "-c") == 0) ||
                    (strcmp(argv[i], "--config"))) {
                    if (argv[i+1] != nullptr) {
                        utils::RecorderConfig::Instance().fastLoadFile(argv[i+1]);
                    }
                }
            }
        }
        return 0;

    }

    interface_t *Server::getSelf(void)
    {
        return &iface;
    }

    void Server::readyReadTcp()
    {

    }

    /// todo:
    /// \brief Server::tcpConnected
    ///
    void Server::tcpConnected()
    {
    }
    /// todo:
    /// \brief Server::tcpDisconnected
    ///
    void Server::tcpDisconnected()
    {
        utils::IPC::Instance().sendMessage("Socket disconnected...\n");
    }

    } // udp
} // plugin

////////////////////////////////////////////////////////////////////////////////
const interface_t *get_interface()
{
    interface_t* piface = plugin::udp::Server::Instance().getSelf();

    piface->init = &plugin::udp::Server::init;
    piface->deinit = &plugin::udp::Server::deinit;
    piface->copy = &plugin::udp::Server::copy;
    piface->put_data = &plugin::udp::Server::put_data;
    piface->put_ndata = &plugin::udp::Server::put_ndata;
    piface->get_data = &plugin::udp::Server::get_data;
    piface->main_proxy = &plugin::udp::Server::p_main;
    piface->getSelf   = &plugin::udp::Server::getSelf;
    piface->setName = &plugin::udp::Server::setName;
    piface->getName = &plugin::udp::Server::getName;
    piface->nextPlugin = nullptr;

    return plugin::udp::Server::Instance().getSelf();
}

