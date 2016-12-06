// std //
#include <iostream> // for test purpose only!

#include "server.h"

namespace iz {

ServerThread::ServerThread(QThread *parent)
    : QThread(parent),
      p_usr(nullptr)
{
}

ServerThread::~ServerThread()
{
    if (p_usr != nullptr) {
        p_usr->deleteLater(); // maybe call this one...
    }
}

void ServerThread::run()
{
    p_usr = new UserServer;
    p_usr->startServer();
    // make it an event loop
    exec();
}

UserServer::UserServer(QObject *parent)
    : QTcpServer(parent),
      p_conn(nullptr)
{
}

UserServer::~UserServer()
{

}

/// starts user`s server
/// \brief UserServer::startServer
///
void UserServer::startServer()
{
    int port = 5678;
    connect(this, SIGNAL(newConnection()),
            this, SLOT(hConnection()));

    // probably i am printing wrong message here...
    if (!this->listen(QHostAddress::Any, port)) {
        std::cout << "Could not start user server\n";
    } else {
        std::cout << "Started user server!\n";
    }
}


/// prints a base menu to the user
/// \brief UserServer::hConnection
///
void UserServer::hConnection()
{
    while (p_conn->canReadLine()) {
        QByteArray line = p_conn->readLine();
        // echo back
        QByteArray resp;
        if (line.contains("help")) {
            resp.append("Help unavailable yet. See you later!\n");
        } else if (line.contains("version")) {
            resp.append("Version... \n");
        } else if (line.contains("quit")) {
            resp.append("BYE!\n");
            p_conn->write(resp);
            p_conn->flush();
            p_conn->waitForBytesWritten();
            p_conn->disconnectFromHost();
            return;
        } else if (line.contains("info")) {

        }
        else {
            resp.append("Unknown command!\n"
                        "Refer to: 'help', 'version', 'info' or 'quit' for now!\n");
        }
        p_conn->write(resp);
        p_conn->flush();
        p_conn->waitForBytesWritten();
    }
}

void UserServer::disconnected()
{
    p_conn->deleteLater();
}

/// TODO: open socket and respond back to user
/// \brief UserServer::incomingConnection
/// \param socketDescriptor
///
void UserServer::incomingConnection(qintptr socketDescriptor)
{
    std::cout << "Connection coming from: (" << socketDescriptor << ")" << std::endl;
    p_conn = new QTcpSocket;
    if (!p_conn->setSocketDescriptor(socketDescriptor)) {
        std::cout << "Can`t setup user response!\n";
    } else {
        connect(p_conn, SIGNAL(readyRead()),
                this, SLOT(hConnection()), Qt::DirectConnection);
        connect(p_conn, SIGNAL(disconnected()),
                this, SLOT(disconnected()));
    }
}

} // namespce iz

