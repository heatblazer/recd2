#include "client.h"
#include <iostream>

#include <math.h>
#include <time.h>

static double* tone_gen(int len)
{
    srand((unsigned)time(NULL));
    double* buff = (double*) malloc(len * (sizeof(double)));
    if (buff) {
        for(int i=0; i < len; ++i) {
            buff[i] = sin(2000 * (2 * M_PI) * i / 6000) + sqrt(0.01) * rand();
        }
        return buff;
    } else {
        return NULL;
    }
}


static double* sine_gen(int len)
{
    double* buff = (double*) malloc(len * sizeof(double));
    if (!buff) {
        return NULL;
    }

    for(int i=0; i < len; ++i) {
        buff[i] = sin(1000 * (2 * M_PI) * i / 44100);
    }

    return buff;
}

short* gen_sawtooth(int len)
{
    short* buff = (short*) malloc(sizeof(short) * len);
    if (!buff) {
        return NULL;
    }

    for(int i=0; i < len; ++i) {
        buff[i] = abs((i++ % len) - (len/2));
    }
    return buff;
}

short* gen_const_tone(int len)
{
    short* buff = new short[len];
    for(int i=0; i < len; ++i) {
        buff[i] = 0;
    }
    return buff;
}

namespace recd {

Client::Client(QObject *parent)
    : QObject(parent),
//      m_addres("192.168.32.154")
//      m_addres("192.168.32.94")
      m_addres("127.0.0.1")
{
    m_timer.setInterval(20);
    connect(&m_timer, SIGNAL(timeout()),
            this, SLOT(transmit()));
}

Client::~Client()
{
}

void Client::init()
{
    p_socket = new QUdpSocket(this);
    p_socket->connectToHost(m_addres,
                            1234, QIODevice::WriteOnly);
    if (p_socket->state() == QUdpSocket::ConnectedState) {
        m_timer.start();
    }

}

void Client::transmit()
{
    std::cout << "Transmitting...\n";
    static uint32_t counter = 0;
    uint16_t* buff = (uint16_t*) gen_sawtooth(16);
    if (buff) {
        for(int i=0; i < 32; ++i) {
            for(int j=0; j < 16; j++) {
                m_packet.packet.data[i][j] = buff[j];
            }
        }

        memset(m_packet.packet.null_bytes, 0, sizeof(m_packet.packet.null_bytes)
                                                / sizeof(m_packet.packet.null_bytes[0]));
        m_packet.packet.counter = ++counter;
        p_socket->write(m_packet.data, sizeof(udp_data_t));
        free(buff);
    } else {
        std::cout << "No data.. \n";
    }
}


} // recd
