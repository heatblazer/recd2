#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QObject>
#include <QQueue>

#include "utils.h"

namespace plugin {
    namespace udp {

    class Server;

    class TcpServer : public QThread
    {
        class Writer : QThread
        {
            Writer(TcpServer* const p);
            static void* worker(void *pArgs);
            void init();
            QMutex lock;
            TcpServer* const ref;
            friend class TcpServer;
            bool m_isRunning;
        };


        static void* worker(void* pArgs);

    public:
        explicit TcpServer();
        virtual ~TcpServer();
        void init();
        void deinit();

    private:
        bool m_isRunning ;
        // if we use other buffer data type...
        struct {
            utils::RingBuffer<utils::frame_data_t, 512> data;
            bool isBusy;
        } m_buffer;

        QMutex m_lock;
        Server* p_server;
        int socket_fd;
        Writer* m_writer;
    };

    } // udp
} // plugin
#endif // TCPSERVER_H
