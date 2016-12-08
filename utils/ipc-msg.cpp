#include "ipc-msg.h"
#include <QObject>

namespace utils {

IPC* IPC::s_inst = nullptr;

IPC &IPC::Instance()
{
    if (s_inst == nullptr) {
        s_inst = new IPC;
    }
    return *s_inst;
}

int IPC::sendMessage(const char *msg)
{
    int wr = p_socket->writeDatagram(QByteArray(msg), QHostAddress::LocalHost, 6666);
    return wr;
}

IPC::IPC(QObject *parent)
    : QObject(parent)
{
    p_socket = new QUdpSocket(this);
}

IPC::~IPC()
{
}


} // utils
