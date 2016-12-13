#include "server.h"
#include <stdio.h>
#include <QCoreApplication>

#include "ipc-msg.h"
#include "date-time.h"

namespace plugin {
    namespace udp {

    /// postimplemented struct for udp header
    /// \brief The udp_data_t struct
    ///
    struct udp_data_t
    {
        uint32_t    counter;
        uint8_t     null_bytes[32];
        int16_t    data[32][16];
    };

    struct sample_data_t
    {
        short* samples;
        uint32_t size;
    };

    Server* Server::s_inst = nullptr;
    interface_t Server::iface = {0,0,0,
                                 0,0,0,
                                 0,0,{0}, // warn fix
                                 0};
    // the err udp packet
    static struct udp_data_t err_udp = {0, {0}, {{0}}}; // warn fix
    ///////////////////////////////////////////////////////////////////////////////////

    Server& Server::Instance()
    {
        if (s_inst == nullptr) {
            s_inst = new Server;
        }
        return *s_inst;
    }

    Server::Server(QObject *parent)
        : QObject(parent)
    {
    }


    /// simple init function
    /// \brief Server::init
    /// \param is_daemon
    /// \param udp maybe and tcp later
    /// \param port
    ///
    void Server::init()
    {
        Server* s = &Server::Instance();
        printf("Initializing server...\n");
        // the error packet to be sent on packet lost
        static const int16_t max = 37222;
        for(int i=0; i < 32; ++i) {
            for(int j=0; j < 16; ++j) {
                err_udp.data[i][j] = max;
            }
        }
        s->udp = new QUdpSocket;
        bool bres = s->udp->bind(1234, QUdpSocket::ShareAddress);

        connect(s->udp, SIGNAL(readyRead()),
                s, SLOT(readyReadUdp())/*, Qt::DirectConnection*/);

        connect(s, SIGNAL(dataReady(udp_data_t*)),
                s, SLOT(hDataReady(udp_data_t*)));
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
    }

    /// ready read datagrams
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
                    udp_data_t* udp = (udp_data_t*) buff.data();

                    // one frame lost for synching with my counter

                    if (udp->counter != ++m_conn_info.paketCounter) {
                        snprintf(msg, sizeof(msg),
                                 "Last synch packet:(%d)\t at: [%s]\n"
                                 "Total desynch:(%d)\n"
                                 "Server counter: (%d)\n"
                                 "Lost: (%d)\n"
                                 "Total lost: (%d)\n",
                                 udp->counter, // next got ocunter
                                 utils::DateTime::getDateTime(),       // current time
                                 m_conn_info.desynchCounter,    // desync counter
                                 m_conn_info.paketCounter,      // server counter
                                 (udp->counter - m_conn_info.paketCounter),  // lost
                                 m_conn_info.totalLost);               // total lost
                        utils::IPC::Instance().sendMessage(msg);
                        m_conn_info.desynchCounter++;
                        int errs = udp->counter - m_conn_info.paketCounter;
                        m_conn_info.totalLost += errs;
                        m_conn_info.paketCounter = udp->counter; // synch back

                        // always write a null bytes packet on missed udp
                        if(!m_conn_info.onetimeSynch) {
                            m_conn_info.onetimeSynch = true;
                            for(int i=0; i < 32; ++i) {
                                put_ndata((short*)err_udp.data[i], 16);
                            }
                        //!!!    put_data((udp_data_t* )&err_udp);
                        } else {
                            for(int i=0; i < errs; ++i) {
                         //!!!       put_data((udp_data_t*) &err_udp);
                               for(int j=0; j < 32; ++j) {
                                   put_ndata((short*)err_udp.data[j], 16);
                               }
                            }
                        }
                    } else {
                    // will use a new logic emit the udp struct
                    // to the recorder, so now we don`t need
                    // to depend each other

                        //put_data((udp_data_t*) udp);
                        QList<sample_data_t> ls;
                        for(int i=0; i < 32; ++i) {
                            sample_data_t s = {0, 0};
                            s.samples = new short[16];
                            s.size = 16;
                            for(int j=0; j < 16; ++j) {
                                s.samples[j] = udp->data[i][j];
                            }
                            ls.append(s);
                        }
                        put_data((QList<sample_data_t>*) &ls);

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

    void Server::hDataReady(udp_data_t *data)
    {
        for(int i=0; i < 32; ++i) {
            put_ndata((short*)&data->data[i], 16);
        }
    }

    void Server::checkConnection()
    {
        static char msg[512] = {0};
        if (m_monitorData.isEmpty()) {
            // not ok!
            Instance().disconnected();
            utils::IPC::Instance().sendMessage("Lost connection!\n");
        } else {
            // make sure you purge the list
            m_monitorData.clear();
        }
        static int counter = 0;
        if (counter > 15) {
            snprintf(msg, sizeof(msg),
                     "Report: \n"
                     "Desynch counter: (%d)\n"
                     "Packet counter: (%d)\n"
                     "Total lost: (%d)\n",
                     Instance().m_conn_info.desynchCounter,
                     Instance().m_conn_info.paketCounter,
                     Instance().m_conn_info.totalLost);
            utils::IPC::Instance().sendMessage(msg);
            counter = 0;
        }
        counter++;
    }

    /// dummy router for future uses of the states
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

    void Server::disconnected()
    {
        m_conn_info.desynchCounter = 0;
        m_conn_info.paketCounter = 0;
        m_conn_info.totalLost = 0;
        m_conn_info.onetimeSynch = false;
    }

    /// deinitialze the server, maybe some unfinished
    /// task to be finalized here, or to be registered in
    /// the daemon, will left it a TODO
    /// \brief Server::deinit
    ///
    void Server::deinit(void)
    {
        utils::IPC::Instance().sendMessage("UDP Server: deinit\n");
    }

    void Server::copy(const void *src, void *dst, int len)
    {
        (void)src; (void) dst; (void) len;
    }

    int Server::put_data(void *data)
    {
        if (iface.nextPlugin != nullptr) {
            iface.nextPlugin->put_data(data);
        }
        return 0;
    }

    int Server::put_ndata(void *data, int len)
    {
        if (iface.nextPlugin != NULL) {
            iface.nextPlugin->put_ndata(data, len);
        }
        return 0;
    }

    void *Server::get_data()
    {
        return NULL; // dummy for now
    }

    int Server::p_main(int argc, char **argv)
    {
        (void) argc;
        (void) argv;
        printf("No args main in server!\n");
        return 0;
    }

    interface_t *Server::getSelf(void)
    {
        return &iface;
    }

    } // udp
} // plugin

///////////////////////////////////////////////////////////////////////////////////
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
    piface->nextPlugin = nullptr;

    return plugin::udp::Server::Instance().getSelf();
}

