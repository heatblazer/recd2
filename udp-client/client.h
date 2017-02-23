#ifndef CLIENT_H
#define CLIENT_H

// qt //
#include <QTimer>
#include <QObject>
#include <QUdpSocket>
#include <QTcpSocket>

#include <stdint.h>
#define PACK_SizE 32 * 4
namespace iz {

#define SMPL_SIZE 100000
struct SMPL {
    char hdr[44];
    short int data[SMPL_SIZE];
};


struct frame_data_t
{
    uint32_t    counter;
    uint8_t     null_bytes[32];
    uint16_t    data[32 * 16];
};

struct frame_data_t2
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
    void disconnected();
    void transmit();
private:
    QTimer m_timer;
    QUdpSocket* p_socket;
    QTcpSocket* p_tcp;
    QHostAddress m_addres;
    union {
        frame_data_t rec_packet;
        char data[sizeof(frame_data_t)];
    } m_packet;

    SMPL file_data;
};

}


#endif // CLIENT_H
