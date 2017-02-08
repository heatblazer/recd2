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
        utils::udp_data_t m_frames;
        QQueue<utils::udp_data_t> m_buffer;
        utils::PMutex m_lock;
        Server* p_server;
        int socket_fd;


    };
    } // udp
} // plugin
#endif // TCPSERVER_H
