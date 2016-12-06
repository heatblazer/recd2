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

// lib //
#include "utils.h"

// custom //
#include "types.h"

namespace iz {

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
