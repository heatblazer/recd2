#include "tcp-server.h"

#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h> // error handlig from the socket fd based on timeout

// utils stuff //
#include "server.h"
#include "utils.h"

#define READ_BUFF_MULTIPLIER(A) ((A) * (sizeof(frame_data_t)))

static const char* THIS_FILE = "tcp-server.cpp";

namespace plugin {

    namespace udp {

    void *TcpServer::worker(void *pArgs)
    {
        int port = (int) Server::Instance().m_port;
        TcpServer* s = (TcpServer*) pArgs;

        static const int SIZE = sizeof(frame_data_t);
        uint8_t* buffer = nullptr;
        int client, newsocketfd;
        size_t len;

        static char msg[128] = {0};
        struct sockaddr_in serv_addr, client_addr;


        if ((s->socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            perror("Error opening socket!");
            return nullptr;
        }

        memset((char*)&serv_addr, 0, sizeof(serv_addr));

        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = INADDR_ANY;
        serv_addr.sin_port = htons(port);

        if (bind(s->socket_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
            perror("BIND failed");
            return  nullptr;
        }

        listen(s->socket_fd, 5);
        client = sizeof(client_addr);

        s->m_writer->init();

        while(s->m_isRunning) {

            newsocketfd = accept(s->socket_fd, (struct sockaddr*)&client_addr, (socklen_t*)&client);

            if (newsocketfd < 0) {
                perror("Error on accept");
                return  nullptr;
            }
            // select on socket filedesc + 1
            int select_result = 0;
            fd_set read_set;
            struct timeval timeout;

            timeout.tv_sec = 5;
            timeout.tv_usec = 0;

            FD_ZERO(&read_set);
            FD_SET(newsocketfd, &read_set);
            select_result = select(newsocketfd+1, &read_set, NULL, NULL, &timeout);

            if (select_result < 0) {
                printf("%s: Error in select()\n",
                       THIS_FILE);
                return nullptr;
            } else if (select_result == 0) {
                printf("%s: select() waited for 5 seconds\n",
                       THIS_FILE);
            } else {

                while (select_result > 0) {
                    frame_data_t frame = {0, {0}, {0}};
                    for(buffer = (uint8_t*)&frame, len = 0; len < SIZE; ) {
                        int nn = read(newsocketfd, buffer, SIZE - len);
                        len += nn;
                        buffer += nn;
                    }
                    if (++s->p_server->m_conn_info.paketCounter != frame.counter) {
                        s->p_server->m_conn_info.paketCounter = frame.counter;
                        snprintf(msg, sizeof(msg), "Missed: %lu\n",
                                 (long unsigned int) s->p_server->m_conn_info.paketCounter+1);
                        utils::IPC::Instance().sendMessage(THIS_FILE, msg);
                    } else {
                        // do soemthin with the data
                        s->m_lock.lock();
                        s->m_buffer.data.write(frame);
                        s->m_lock.unlock();
                    }
                }
                // close old conn - register a new one
                close(newsocketfd);
            }
        } // end forever loop

        snprintf(msg, sizeof(msg), "%s: Stopping TCP server.\n", THIS_FILE);
        utils::IPC::Instance().sendMessage(THIS_FILE, msg);

        // stop writer thread
        s->m_writer->m_isRunning = false;        
        // finally close socket
        close(s->socket_fd);
        return nullptr;
    }

    TcpServer::TcpServer()
        :
          m_isRunning(false),
          p_server(nullptr),
          socket_fd(-1)
    {
        p_server = &Server::Instance();
        m_writer = new Writer(this);
        m_buffer.data.init();
    }

    TcpServer::~TcpServer()
    {
    }

    void TcpServer::init()
    {
        // start buffer thread
        m_isRunning = true;
        setObjectName(QString("tcp-worker-thread"));
        start();
    }

    void TcpServer::deinit()
    {
        utils::IPC::Instance().sendMessage(THIS_FILE, "TCP server deinit\n");
        m_isRunning = false;
    }

    TcpServer::Writer::Writer(TcpServer * const p)
        : ref(p),
          m_isRunning(false)
    {
    }

    void *TcpServer::Writer::worker(void *pArgs)
    {
        Writer* w  = (Writer*) pArgs;
        static const unsigned MAX = w->ref->p_server->m_channels * w->ref->p_server->m_smplPerChan;
        for (;;) {
            // perform the read stuff here
            QList<sample_data_t> ls;
            frame_data_t* t = nullptr;
            w->lock.lock();
            t = (w->ref->m_buffer.data).read();
            w->lock.unlock();
            if (t == nullptr) {
                continue;
            } else {
                printf("Cnt: %u\r", t->counter);
                for(unsigned i=0; i < MAX; ) {
                    sample_data_t sdata = {{0}, 0, 0};
                    short smpl[512] = {0};
                    sdata.samples = smpl;
                    sdata.size = w->ref->p_server->m_smplPerChan;
                    for (uint32_t j=0; j < sdata.size; ++j) {
                        sdata.samples[j] = t->data[i++];
                    }
                    ls.append(sdata);
                }
                Server::Instance().put_data((QList<sample_data_t>*)&ls);
            }
        } // forever loop

        // at end read all from rbuffer
        frame_data_t* rem = nullptr;
        QList<sample_data_t> ls;
        int s = w->ref->m_buffer.data.readAll(&rem);
        for(int i=0; i < s; ++i) {
            for(unsigned ii=0; ii < MAX; ) {
                sample_data_t sdata = {{0}, 0, 0};
                short smpl[512] = {0};
                sdata.samples = smpl;
                sdata.size = w->ref->p_server->m_smplPerChan;;
                for (uint32_t j=0; j < sdata.size; ++j) {
                    sdata.samples[j] = rem[i].data[ii++];
                }
                ls.append(sdata);
            }
        }
        Server::Instance().put_data((QList<sample_data_t>*)&ls);
        delete [] rem;
        return nullptr;
    }

    void TcpServer::Writer::init()
    {
        setObjectName("tcp-writer");
        m_isRunning = true;
        start();
    }

    } // udp
} // plugin
