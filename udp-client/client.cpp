#include "client.h"
#include <iostream>

#include <math.h>
#include <time.h>
#include <stdio.h>

#define CHANS 32
#define SAMPLES 16
static short* test_gen(int len)
{
    short* s = new short[len];
    if (!s) {
        return nullptr;
    }

    for(int i=0; i < len; ++i) {
        s[i] = (i % 2==0) ? 12345 : 0;
    }
    return s;
}


namespace iz {

Client::Client(QObject *parent)
    : QObject(parent),
//      m_addres("192.168.32.154")
//      m_addres("192.168.32.94")
      m_addres("127.0.0.1")
{
}

Client::~Client()
{
}

void Client::init()
{
    // load file into memory
    {
        FILE* fp = fopen("test.wav", "rb");
        if (!fp) {
            return;
        }
        size_t r = fread(&file_data, 1, sizeof(SMPL), fp);
        fclose(fp);
    }

    m_timer.setInterval(1);
    connect(&m_timer, SIGNAL(timeout()),
    this, SLOT(transmit()));

    p_socket = new QUdpSocket(this);

    p_socket->connectToHost(QHostAddress::LocalHost, 1234, QIODevice::WriteOnly);
    if (p_socket->state() == QUdpSocket::ConnectedState) {
            m_timer.start();
    }

}

void Client::disconnected()
{
    std::cout << "Disconnected!\n";
}

void Client::transmit()
{
    static uint32_t counter = 0;
    std::cout << "Transmitting...\n";
    static unsigned int file_cnt = 0;
    for (int i=0; i < CHANS * SAMPLES; ) {
        for(int j=0; j < SAMPLES; ++j) {
            m_packet.rec_packet.data[i++] = file_data.data[file_cnt++ % 100000];
        }
    }

    memset(m_packet.rec_packet.null_bytes, 0, sizeof(m_packet.rec_packet.null_bytes)
                                             / sizeof(m_packet.rec_packet.null_bytes[0]));
    m_packet.rec_packet.counter = ++counter;
    p_socket->write(m_packet.data, sizeof(frame_data_t));
}


} // iz
