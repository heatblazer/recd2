#ifndef IPCMSG_H
#define IPCMSG_H

#include <QObject>
#include <QUdpSocket>

namespace utils {

// message queue
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
