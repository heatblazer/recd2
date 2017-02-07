#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QObject>
#include <QQueue>

#include "utils.h"


namespace plugin {
    namespace udp {

    class Server;

    struct sample_data_t
    {
        short* samples;
        uint32_t size;
    };


    struct udp_data_t
    {
        uint32_t    counter;
        uint8_t     null_bytes[32];
        int16_t     data[32][16];

    };


    class TcpServer : public utils::PThread
    {
        static void* worker(void* pArgs);
    public:
        explicit TcpServer();
        virtual ~TcpServer();
        void init();
        void deinit();

    private:
        QQueue<udp_data_t> m_buffer;
        Server* p_server;
        int socket_fd;
        utils::SpinLock m_lock;

    };
    } // udp
} // plugin
#endif // TCPSERVER_H
