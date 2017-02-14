#include "server.h"

#include <iostream>


#include <QCoreApplication>

#include "ipc-msg.h"
#include "date-time.h"
#include "recorder-config.h"
#include "types.h"

namespace plugin {
    namespace udp {

    /// flip the bytes to match the logic of the program
    /// \brief copy_and_flip
    /// \param in
    /// \return
    ///
    static inline utils::frame_data_t copy_and_flip(utils::frame_data_t2 in)
    {
        utils::frame_data_t out;
        out.counter = in.counter;
        for(int i=0; i < 32; ++i) {
            out.null_bytes[i] = in.null_bytes[i];
        }

        for(int i=0; i < 16; ++i) {
            for(int j=0; j < 32; ++j) {
                out.data[j][i] = in.data[i][j];
            }
        }
        return out;
    }


    Server* Server::s_inst = nullptr;
    interface_t Server::iface = {0,0,0,
                                 0,0,0,
                                 0,0,0,
                                 0,{0},0};
    // the err udp packet
    static struct utils::frame_data_t err_udp = {0,{0},{{0}}}; // warn fix
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
          m_conn_info{0,0,0,false}
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
        static const int16_t max = 32111;
        for(int i=0; i < 32; ++i) {
            for(int j=0; j < 16; ++j) {
                err_udp.data[i][j] = max;
            }
        }
        bool bres = false;

        const utils::MPair<QString, QString> transport =
                utils::RecorderConfig::Instance().
                getAttribPairFromTag("Network", "transport");
        if (transport.m_type2 == "udp") {
                s->udp = new QUdpSocket;
                bres = s->udp->bind(1234, QUdpSocket::ShareAddress);

                connect(s->udp, SIGNAL(readyRead()),
                        s, SLOT(readyReadUdp())/*, Qt::DirectConnection*/);

                connect(s, SIGNAL(dataReady(frame_data_t*)),
                        s, SLOT(hDataReady(frame_data_t*)));
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
            } else if (transport.m_type2 == "tcp") {
                // give time to everithying to init
                QTimer::singleShot(0,
                                   [=]()
                {
                    s->initTcpServer();
                });

        } else {
            std::cout << "Invalid xml attribute (" << transport.m_type2.toStdString()
                      << ")" << " from Tag: ("  <<
                         transport.m_type1.toStdString() << ")" << std::endl;
            exit(1);
        }

    }

    /// ready read datagrams, send to other plugins
    /// perform some packet reciever checks and other stuff
    /// \brief Server::readyReadUdp
    ///
    void Server::readyReadUdp()
    {
        static char msg[512] = {0};

        if (udp->hasPendingDatagrams()) {
            m_monitorData.append('.');

            while (udp->hasPendingDatagrams()) {

                QByteArray buff;
                buff.resize(udp->pendingDatagramSize());

                qint64 read = udp->readDatagram(buff.data(), buff.size(),
                                       &m_senderHost, &m_senderPort);
                if (read > 0) {
                    // the udp structure from the device
#ifdef REQ_FLIP
                    utils::frame_data_t2* udp2 = (utils::frame_data_t2*) buff.data();
                    utils::frame_data_t udp = copy_and_flip(*udp2);
#else
                    frame_data_t udp = *(frame_data_t*) buff.data();
#endif
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
                            for(int i=0; i < 32; ++i) {
                                utils::sample_data_t s = {0, 0};
                                short smpl[16] = {0};
                                s.samples = smpl;
                                s.size = 16;
                                for(int j=0; j < 16; ++j) {
                                    s.samples[j] = err_udp.data[i][j];
                                }
                                err_ls.append(s);
                            }
                            put_data((QList<utils::sample_data_t>*) &err_ls);
                        } else {
                            for(int i=0; i < errs; ++i) {
                               for(int j=0; j < 32; ++j) {
                                   utils::sample_data_t sd = {0, 0};
                                   short smpls[16]= {0};
                                   sd.samples = smpls;
                                   sd.size = 16;
                                   for(int h=0; h < 16; ++h) {
                                       sd.samples[h] = err_udp.data[j][h];
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
                        for(int i=0; i < 32; ++i) {
                            utils::sample_data_t s = {0, 0};
                            short smpls[16] ={0};
                            s.samples = smpls;
                            s.size = 16;
                            // fill the list to be passed to other plugins
                            for(int j=0; j < 16; ++j) {
                                s.samples[j] = udp.data[i][j];
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

    /// unused
    /// \brief Server::hDataReady
    /// \param data
    ///
    void Server::hDataReady(utils::frame_data_t *data)
    {

        for(int i=0; i < 32; ++i) {
            put_ndata((short*)&data->data[i], 16);
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
        s->p_server->deinit();

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

#if 0
    // tpc server stuff
    TcpServer::TcpServer()
        :
         m_packet{0},
         socket_fd(-1)
    {
    }

    TcpServer::~TcpServer()
    {

    }

    void TcpServer::init()
    {
#if 0
        tcp_server = new QTcpServer(this);
        tcp_server->setMaxPendingConnections(1);

        connect(tcp_server, SIGNAL(newConnection()),
                this, SLOT(hConnection()));
        if (!tcp_server->listen(QHostAddress::Any, 1234)) {
            utils::IPC::Instance().sendMessage("Server failed to start!\n");
        } else {
            utils::IPC::Instance().sendMessage("Server started!\n");
        }
#endif

    }

    void TcpServer::hConnection()
    {

#if 0
        QTcpSocket* sock = tcp_server->nextPendingConnection();
        ((QAbstractSocket*) sock)->setSocketOption(QAbstractSocket::LowDelayOption, 1);
        //((QAbstractSocket*) sock)->setSocketOption(QAbstractSocket::ReceiveBufferSizeSocketOption,
        //                                           1060 * 10);

        if (sock != nullptr) {
            connect(sock, SIGNAL(readyRead()),
                    this, SLOT(readyReadData()));

            //connect(sock, SIGNAL(stateChanged(QAbstractSocket::SocketState)),
            //        this, SLOT(hState(QAbstractSocket::SocketState)));
            //connect(sock, SIGNAL(disconnected()),
            //        sock, SLOT(deleteLater()));

        }
#endif
    }

    void TcpServer::hDisconnect()
    {
        std::cout << "Disconnected from client..." << std::endl;
        exit(1);
    }

    void TcpServer::hState(QAbstractSocket::SocketState state)
    {
        switch (state) {
        case QAbstractSocket::ConnectedState:
            utils::IPC::Instance().sendMessage("Connected\n");
            break;
        case QAbstractSocket::ConnectingState:
            utils::IPC::Instance().sendMessage("Connecting\n");
            break;
        case QAbstractSocket::UnconnectedState:
            utils::IPC::Instance().sendMessage("Not connected\n");
            break;
        default:
            utils::IPC::Instance().sendMessage("Unknown \n");
            break;
        }
    }


    void TcpServer::readyReadData()
    {
        Server* s = &Server::Instance();

        QTcpSocket* r = static_cast<QTcpSocket*>(sender());

        if (r == nullptr) {
            std::cout << "Failed to obtain a socket reference." << std::endl;
            exit(0); // deleteme later
        }

        int drainBytes = sizeof(frame_data_t) - m_packet.size;
        if (drainBytes != 0) {
            QByteArray d = r->read(drainBytes);
            m_packet.size += d.count();
            m_packet.data.append(d.data(), d.count());
        }

        if ((drainBytes == 0) && (m_packet.size == sizeof(frame_data_t))) {

            // the udp structure from the device
            frame_data_t tcp = *(frame_data_t*) m_packet.data.data();
            std::cout << tcp.counter << std::endl;

            if (++s->m_conn_info.paketCounter != tcp.counter) {
                s->m_conn_info.paketCounter = tcp.counter;
                // missed a packet
                static char buff[64] = {0};
                snprintf(buff, sizeof(buff), "Missed:%lu\n", s->m_conn_info.paketCounter-1);
                utils::IPC::Instance().sendMessage(buff);

            }
            QList<sample_data_t> ls;
            // copy all the data then send it to the plugins
            for(int i=0; i < 32; ++i) {
                sample_data_t s = {0, 0};
                short smpls[16] ={0};
                s.samples = smpls;
                s.size = 16;
                // fill the list to be passed to other plugins
                for(int j=0; j < 16; ++j) {
                    s.samples[j] = tcp.data[i][j];
                }
                ls.append(s);
            }
            // finally send it
            s->put_data((QList<sample_data_t>*) &ls);
            m_packet.data.clear();
            m_packet.size = 0;
        }

    }
#endif
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

