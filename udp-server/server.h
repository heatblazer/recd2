#ifndef SERVER_H
#define SERVER_H

// qt //
#include <QFile>
#include <QObject>
#include <QTimer>
#include <QThread>
#include <QTcpSocket>
#include <QTcpServer> // usr server
#include <QUdpSocket>

// custom //
#include "types.h"
#include "utils/writer.h"
#include "utils/wav-writer.h"

namespace iz {

class SApplication;
class ServerThread;
class UserServer;

class Server : public QObject
{
    Q_OBJECT
public:
    // for future uses
    enum States {
                UNKNOWN=0,
                DISCONNECTED = 1,
                CONNECTED = 2,
                LOST_CONNECTION = 3,
                GOT_CONNECTION = 4,
                GOT_DATAGRAM = 5,
                MISSED_DATAGRAM = 6
               };

    explicit Server(QObject* parent=nullptr);
    virtual ~Server();
    void init(bool udp=true, quint16 port=1234, bool send_heart=false);
    void deinit();

signals:
    void dataReady(QQueue<udp_data_t>& packets);
    void dataReady(const udp_data_t& data);
    void dataReady(const tcp_data_t& data);

private slots:
    void readyReadUdp();
    void handleConnection();
    void hStateChange(QAbstractSocket::SocketState state);
    void error(QAbstractSocket::SocketError err);
    void checkConnection();

    void route(States state);
    void disconnected();
    void sendHeartbeat();

private:
    union {
        QUdpSocket* udp;
        QTcpSocket* tcp; // pending deprecation...
    } m_socket;


    QUdpSocket* m_hearSocket;
    QTimer      m_heartbeat;
    QTimer      m_liveConnection;
    QHostAddress m_senderHost;
    quint16      m_senderPort;
    bool        m_sendHeart; // inspired by...

    struct {
        uint32_t paketCounter;
        uint32_t desynchCounter;
        uint32_t totalLost;
        bool     onetimeSynch;
    } m_conn_info;

    // for the hotswap I may need an auxilary buffer
    // to store the udp packets while closing and creating
    // the new wav files, so nothing will be lost, I`ll do
    // that logic later.

    // may be the client will send me backup packets
    // let`s say 5 so I must store a 5 elements array
    // and to append the  firs occurency I meet, then
    // move to antoher buffer, new concept, unimplemented

    QQueue<char> m_monitorData;

    friend class UserServer;
};

///////////////////////////////////////////////////////////////////////////////////


class UserServer : public QTcpServer
{
    Q_OBJECT
    explicit UserServer(QObject* parent = nullptr);
    virtual ~UserServer();
    void startServer();
private slots:
    void hConnection();
    void disconnected();
protected:
    void incomingConnection(qintptr socketDescriptor);
    QTcpSocket*         p_conn;

    friend class ServerThread;
};

class ServerThread : public QThread
{
    Q_OBJECT
    explicit ServerThread(QThread* parent = nullptr);
    virtual ~ServerThread();
    virtual void run();

private slots:

private:
    UserServer* p_usr;
    friend class SApplication;
};


} // namespace iz

#endif // SERVER_H
