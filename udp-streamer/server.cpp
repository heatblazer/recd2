#include "server.h"
#include <stdio.h>
#include <QCoreApplication>
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

///////////////////////////////////////////////////////////////////////////////////
QUdpSocket* Server::udp = nullptr;
QTimer Server::m_liveConnection;
QHostAddress Server::m_senderHost;
quint16 Server::m_senderPort = 1234;
struct conn_info Server::m_conn_info = {0, 0, 0, false};
QQueue<char> Server::m_monitorData;
Server* Server::s_inst = nullptr;
interface_t Server::iface = {0,0,0,
                             0,0,0,
                             0,0,0};

// the err udp packet
static struct udp_data_t err_udp = {0, 0, 0};
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
    printf("Initializing server...\n");
    // the error packet to be sent on packet lost
    for(int i=0; i < 32; ++i) {
        for(int j=0; j < 16; ++j) {
            err_udp.data[i][j] = 37222;
        }
    }

    udp = new QUdpSocket;
    bool bres = udp->bind(1234, QUdpSocket::ShareAddress);

    connect(udp, SIGNAL(readyRead()),
            &Instance(), SLOT(readyReadUdp())/*, Qt::DirectConnection*/);

    connect(&Instance(), SIGNAL(dataReady(udp_data_t*)),
            &Instance(), SLOT(hDataReady(udp_data_t*)));
    if (bres) {

        printf("Bind OK!\n");
        m_liveConnection.setInterval(1000);
        connect(&m_liveConnection, SIGNAL(timeout()),
                &Instance(), SLOT(checkConnection()));
        m_liveConnection.start();

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
    // write error udp to prevent wav size
    // fragmenation, if missed an udp,
    // I`ll write a 16 samples with max valuse
    printf("Ready read udp data!\n");

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
                    printf("Last synch packet:(%d)\t at: [%s]\n"
                             "Total desynch:(%d)\n"
                             "Server counter: (%d)\n"
                             "Lost: (%d)\n"
                             "Total lost: (%d)\n",
                             udp->counter, // next got ocunter
                             DateTime::getDateTime(),       // current time
                             m_conn_info.desynchCounter,    // desync counter
                             m_conn_info.paketCounter,      // server counter
                             (udp->counter - m_conn_info.paketCounter),  // lost
                             m_conn_info.totalLost);               // total lost

                    m_conn_info.desynchCounter++;
                    int errs = udp->counter - m_conn_info.paketCounter;
                    m_conn_info.totalLost += errs;
                    m_conn_info.paketCounter = udp->counter; // synch back

                    // always write a null bytes packet on missed udp
                    if(!m_conn_info.onetimeSynch) {
                        m_conn_info.onetimeSynch = true;
                        emit Instance().dataReady(&err_udp);

                    } else {
                        for(int i=0; i < errs; ++i) {
                            emit Instance().dataReady(&err_udp);
                        }
                    }

                } else {
                // will use a new logic emit the udp struct
                // to the recorder, so now we don`t need
                // to depend each other
                    emit Instance().dataReady(udp);
                }

             } else {
                printf("Missed an UDP!\n");
            }
        }

    } else {
        Instance().disconnected();
    }
}

void Server::hDataReady(udp_data_t *data)
{
    for(int i=0; i < 32; ++i) {
        put_ndata((udp_data_t*)&data[i], 16);
    }
}

void Server::checkConnection()
{
    if (m_monitorData.isEmpty()) {
        // not ok!
        Instance().disconnected();
    } else {
        // make sure you purge the list
        m_monitorData.clear();
    }
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
}

void Server::copy(const void *src, void *dst, int len)
{
    (void)src; (void) dst; (void) len;
}

int Server::put_data(void *data)
{
    printf("Server: put data to ");
    if (iface.nextPlugin != NULL) {
        puts("next plugin.");
        iface.nextPlugin->put_data(data);
    } else {
        puts("no one.");
    }
    return 0;
}

int Server::put_ndata(void *data, int len)
{
    printf("Server: put data to ");
    if (iface.nextPlugin != NULL) {
        puts("next plugin.");
        iface.nextPlugin->put_ndata(data, len);
    } else {
        puts("no one.");
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
