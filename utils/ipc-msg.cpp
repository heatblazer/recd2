#include "ipc-msg.h"
#include <QObject>

namespace utils {

IPC* IPC::s_inst = nullptr;

/// it`s singleton anyway
/// \brief IPC::Instance
/// \return the instance
///
IPC &IPC::Instance()
{
    if (s_inst == nullptr) {
        s_inst = new IPC;
    }
    return *s_inst;
}

/// TODO: configure the port and host, host may remain localhost
/// \brief IPC::sendMessage
/// \param msg - message to be sent
/// \return bytes written
///
int IPC::sendMessage(const char *module, const char *msg)
{
    char module_msg[512] = {0};
    snprintf(module_msg, sizeof(module_msg), "[%s]:%s", module, msg);
    int wr = p_socket->writeDatagram(QByteArray(module_msg), QHostAddress::LocalHost, 6666);
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
