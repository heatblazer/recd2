#ifndef CLIENT_H
#define CLIENT_H

// qt //
#include <QTimer>
#include <QObject>
#include <QUdpSocket>

#include <stdint.h>
#define PACK_SIZE 32 * 4
namespace iz {
struct udp_data_t
{
    uint32_t    counter;
    uint8_t     null_bytes[32];
    uint16_t    data[16][32];
};
class Client : public QObject
{
    Q_OBJECT
public:
    explicit Client(QObject* parent=nullptr);
    ~Client();
    void init();

public slots:

    void transmit();
private:
    QTimer m_timer;
    QUdpSocket* p_socket;
    QHostAddress m_addres;
    union {
        udp_data_t packet;
        char data[sizeof(udp_data_t)];
    } m_packet;
};

}


#endif // CLIENT_H
