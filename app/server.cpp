#include "server.h"

#include <iostream> // for test purpose only!

static const char* THIS_FILE = "server.cpp";

namespace recd {

ServerThread::ServerThread(QThread *parent)
    : QThread(parent),
      p_msg(nullptr),
      p_usr(nullptr)
{
}

ServerThread::~ServerThread()
{
    if (p_usr != nullptr) {
        p_usr->deleteLater(); // maybe call this one...
    }

    if (p_msg != nullptr) {
        p_msg->deinit();
        p_msg->deleteLater();
    }
}

void ServerThread::run()
{
    p_msg = new MsgServer;
    p_msg->init();

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

MsgServer::MsgServer(QObject *parent)
    : QObject(parent)
{

}

MsgServer::~MsgServer()
{

}

void MsgServer::init()
{
    std::cout << "Starting message server..." << std::endl;
    p_server = new QUdpSocket(this);
    const bool res = p_server->bind(6666, QUdpSocket::ShareAddress);

    connect(p_server, SIGNAL(readyRead()),
            this, SLOT(readyRead()));

    if (!res) {
        // handle later
        utils::Logger::Instance().logMessage(THIS_FILE, "Message server failed!!!\n");
        utils::Logger::Instance().logMessage(THIS_FILE, "It`s a critical error!!!\n");
        std::cout << "Message server failed!\n";
    } else {
        std::cout << "Message server started. Listeing to 6666 :) \n";
        utils::Logger::Instance().logMessage(THIS_FILE, "Bind message server: OK!\n");
    }

}

void MsgServer::deinit()
{
 // nothing for now... maybe message later
}

void MsgServer::readyRead()
{
    if (p_server->hasPendingDatagrams()) {
        while (p_server->hasPendingDatagrams()) {
            QByteArray msg;
            msg.resize(p_server->pendingDatagramSize());
            quint64 read = p_server->readDatagram(msg.data(), msg.size());
            if (read > 0) {
                utils::Logger::Instance().logMessage(THIS_FILE, msg);
            } else {
                // write err msg!
            }
        }
    }
}

} // namespce recd

