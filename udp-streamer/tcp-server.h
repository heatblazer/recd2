#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QObject>
#include <QQueue>

#include "utils.h"


namespace plugin {
    namespace udp {

    class Server;


    class TcpServer : public utils::PThread
    {
        class Writer : utils::PThread
        {
            Writer(TcpServer* const p);
            static void* worker(void *pArgs);
            void init();
            utils::PMutex lock;
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
        utils::frame_data_t m_frames;
        struct {
            utils::RingBuffer data;
            bool isBusy;
        } m_buffer;
        utils::PMutex m_lock;
        Server* p_server;
        int socket_fd;
        Writer* m_writer;
    };

    } // udp
} // plugin
#endif // TCPSERVER_H
