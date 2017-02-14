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

#define READ_BUFF_MULTIPLIER(A) ((A) * (sizeof(utils::frame_data_t)))

namespace plugin {
namespace udp {


void *TcpServer::worker(void *pArgs)
{
    TcpServer* s = (TcpServer*) pArgs;

    static const int SIZE = sizeof(utils::frame_data_t);
    uint8_t* buffer = nullptr;
    int client, newsocketfd;
    size_t len;

    static char msg[128] = {0};
    struct sockaddr_in serv_addr, client_addr;


    if ((s->socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Error opening socket!");
        exit(1);
    }

    memset((char*)&serv_addr, 0, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(1234);

    if (bind(s->socket_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("BIND failed");
        exit(1);
    }

    listen(s->socket_fd, 5);
    client = sizeof(client_addr);

    s->m_writer->init();


    while(s->m_isRunning) {

        newsocketfd = accept(s->socket_fd, (struct sockaddr*)&client_addr, (socklen_t*)&client);

        if (newsocketfd < 0) {
            perror("Error on accept");
            exit(1);
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
            printf("Error in select()\n");
            //utils::IPC::Instance().sendMessage("Error in select()\n");
            exit(1);
        } else if (select_result == 0) {
            printf("select() waited for 5 seconds\n");
            //utils::IPC::Instance().sendMessage("select timed out after 20 sec\n");
        } else {

            while (select_result > 0) {
                utils::frame_data_t frame = {0, {0}, {{0}}};
                for(buffer = (uint8_t*)&frame, len = 0; len < SIZE; ) {
                    int nn = read(newsocketfd, buffer, SIZE - len);
                    len += nn;
                    buffer += nn;
                }
                if (++s->p_server->m_conn_info.paketCounter != frame.counter) {
                    s->p_server->m_conn_info.paketCounter = frame.counter;
                    snprintf(msg, sizeof(msg), "Missed: %lu\n",
                             (long unsigned int) s->p_server->m_conn_info.paketCounter+1);
                    utils::IPC::Instance().sendMessage(msg);
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
    }
    utils::IPC::Instance().sendMessage("Stoppint TCP server...\n");
    // finally close socket
    s->m_writer->m_isRunning = false;
    s->m_writer->join();

    close(s->socket_fd);
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
    m_lock.init();
    setName("tcp-thread");
    m_isRunning = true;
    create(128 * 1024, 20, TcpServer::worker, this);
}

void TcpServer::deinit()
{
    utils::IPC::Instance().sendMessage("TCP server deinit\n");
    m_isRunning = false;
    join();
    m_lock.deinit();
}

TcpServer::Writer::Writer(TcpServer * const p)
    : ref(p),
      m_isRunning(false)
{
}

void *TcpServer::Writer::worker(void *pArgs)
{
    Writer* w  = (Writer*) pArgs;

    for (;;) {
        // perform the read stuff here
        QList<utils::sample_data_t> ls;
        utils::frame_data_t* t = nullptr;
        w->lock.lock();
        t = (w->ref->m_buffer.data).read();
        w->lock.unlock();
        if (t == nullptr) {
            continue;
        } else {
            printf("Cnt: %lu\r", t->counter);
            for(int i=0; i < 32; ++i) {
                utils::sample_data_t sdata = {0, 0};
                short samples[16] = {0};
                sdata.samples = samples;
                sdata.size = 16;
                for (int j=0; j < 16; ++j) {
                    sdata.samples[j] = t->data[i][j];
                }
                ls.append(sdata);
            }
            Server::Instance().put_data((QList<utils::sample_data_t>*)&ls);
        }
    }
}

void TcpServer::Writer::init()
{
    lock.init();
    setName("tcp-writer");
    m_isRunning = true;
    create(128 * 1024, 10, Writer::worker, this);
}


} // udp

} // plugin
