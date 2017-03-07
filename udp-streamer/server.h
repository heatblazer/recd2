#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QTimer>
#include <QTcpSocket>
#include <QTcpServer>
#include <QThread>
#include <QUdpSocket>
#include <QQueue>

#include <stdint.h>

#include "plugin-interface.h"
#include "tcp-server.h"
#include "utils.h"

#include "helper.h"

namespace plugin {
    namespace udp {

    struct conn_info {
        uint32_t paketCounter;
        uint32_t desynchCounter;
        uint32_t totalLost;
        bool     onetimeSynch;
    } ;


    struct PeekOptions
    {
        bool peekOnOff;
        int peekSize;
    } ;

    class Server : public QObject
    {
        Q_OBJECT
    // interface implementation
    public:

        static PeekOptions s_peekOptions;

        enum Sizes
        {
            MaxSampleSize = 512,
            MinChanSize = 1
        };

        static Server& Instance();
        static void init();
        static void deinit(void);
        static void copy(const void* src, void* dst, int len);
        static int put_data(void* data);
        static int put_ndata(void* data, int len);
        static void* get_data(void);
        static void setName(const char* name);
        static const char* getName(void);
        static int p_main(int argc, char** argv);
        static struct interface_t* getSelf(void);

    private:
        enum States {
                    UNKNOWN=0,
                    DISCONNECTED = 1,
                    CONNECTED = 2,
                    LOST_CONNECTION = 3,
                    GOT_CONNECTION = 4,
                    GOT_DATAGRAM = 5,
                    MISSED_DATAGRAM = 6
        };

        static struct interface_t iface;

    signals:
        void dataReady(utils::frame_data_t* data);

    private slots:
        void readyReadUdp();
        void readyReadTcp();
        void tcpConnected();
        void tcpDisconnected();
        void checkConnection();
        void route(States state);
        void disconnected();
        void initTcpServer();

    private:
        static utils::frame_data_t err_udp;
        explicit Server(QObject* parent=nullptr);
        virtual ~Server();

        Helper* m_helper;


        QUdpSocket* udp;
        TcpServer*  p_server;

        QTimer      m_liveConnection;
        QHostAddress m_senderHost;
        quint16      m_senderPort;
        conn_info m_conn_info;
        QQueue<char> m_monitorData;
        static Server* s_inst;
        unsigned int m_channels;
        unsigned int m_smplPerChan;
        quint16 m_port;



        friend class TcpServer;
        friend class ServerThread;
        friend class Helper;
    };
    } // udp
} // plugin

#endif // SERVER_H
