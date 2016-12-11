#ifndef SERVER_H
#define SERVER_H

#include <QFile>
#include <QObject>
#include <QTimer>
#include <QThread>
#include <QTcpSocket>
#include <QTcpServer> // usr server
#include <QUdpSocket>

#include "utils.h"
#include "types.h"

namespace iz {

/// used for the logging system
/// all plugins and outside stuff will send
/// messages here, I don`t want to block it
/// so it`s udp
/// \brief The MsgServer class
///
class MsgServer : public QObject
{
    Q_OBJECT // this class emits
public:
    explicit MsgServer(QObject* parent=nullptr);
    virtual ~MsgServer();
    void init();
    void deinit();

public slots:
    void readyRead();

private:
    QUdpSocket* p_server;
};

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
    MsgServer* p_msg;
    UserServer* p_usr;
    friend class SApplication;
};


} // namespace iz

#endif // SERVER_H
