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

namespace iz {

Client::Client(QObject *parent)
    : QObject(parent),
//      m_addres("192.168.32.154")
//      m_addres("192.168.32.94")
      m_addres("127.0.0.1")
{
    m_timer.setInterval(5);
    connect(&m_timer, SIGNAL(timeout()),
            this, SLOT(transmit()));
}

Client::~Client()
{
}

void Client::init()
{
    p_tcp = new QTcpSocket(this);
    connect(p_tcp, SIGNAL(connected()),
            this, SLOT(transmit()));
    connect(p_tcp, SIGNAL(disconnected()),
            this, SLOT(disconnected()));
    p_tcp->connectToHost(QHostAddress::Any,
                          1234);
    if (!p_tcp->waitForConnected(3000)) {

    } else {
        while (1) {
            transmit();
        }
    }

}

void Client::disconnected()
{
    std::cout << "Disconnected!\n";
}

void Client::transmit()
{    static uint32_t counter = 0;
     for(int i=0; i < 16; ++i) {
         for(int j=0; j < 32; j++) {
             m_packet.packet.data[i][j] = 1;
         }
     }

     memset(m_packet.packet.null_bytes, 0, sizeof(m_packet.packet.null_bytes)
                                             / sizeof(m_packet.packet.null_bytes[0]));
     m_packet.packet.counter = ++counter;
     if (m_packet.packet.counter > 200000000) {
         int i = 0;
     }
     p_tcp->write(m_packet.data, sizeof(udp_data_t));
     p_tcp->flush();
     std::cout << "Transmitting...\n";

}


} // iz
