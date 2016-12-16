#ifndef IPCMSG_H
#define IPCMSG_H

#include <QObject>
#include <QUdpSocket>

namespace utils {

// message queue

/// send messages via udp packets, to prevent
/// expensive calls from multiple logger instances
/// msgget, msgsend are not proper since, messages
/// can be get anytime, and messages from old programs
/// may arive on fresh boot of the program.
/// \brief The IPC class
///
class IPC : public QObject
{
public:
    static IPC& Instance();
    int sendMessage(const char* msg);
private:
    explicit IPC(QObject* parent=nullptr);
    virtual ~IPC();
    QUdpSocket* p_socket;
    static IPC* s_inst;
};

} // utils
#endif // IPCMSG_H
