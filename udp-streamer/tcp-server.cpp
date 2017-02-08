#include "tcp-server.h"

#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>

#include "server.h"
#include "utils.h"

#define READ_BUFF_MULTIPLIER(A) ((A) * (sizeof(utils::udp_data_t)))

namespace plugin {
namespace udp {


void *TcpServer::worker(void *pArgs)
{
    TcpServer* s = (TcpServer*) pArgs;

    QQueue<utils::udp_data_t> dblBff;

    for(;;) {

        s->m_lock.lock();
        dblBff = s->m_buffer;
        s->m_lock.unlock();

        QList<sample_data_t> ls;
        if (dblBff.isEmpty()) {
            continue;
        }
        while (!dblBff.isEmpty()) {
            const utils::udp_data_t& frame = s->m_buffer.dequeue();
            std::cout << frame.counter << std::endl;
            // copy all the data then send it to the plugins

            for(int i=0; i < 32; ++i) {
                sample_data_t pkt = {0, 0};
                short smpls[16] ={0};

                pkt.samples = smpls;
                pkt.size = 16;
                // fill the list to be passed to other plugins
                for(int j=0; j < 16; ++j) {
                    pkt.samples[j] = frame.data[i][j];
                }
                ls.append(pkt);
            }
        }
        // finally send it
        s->p_server->put_data((QList<sample_data_t>*) &ls);
        s->suspend(0);
    }
}

TcpServer::TcpServer()
    :
      p_server(nullptr),
      socket_fd(-1)
{
    p_server = &Server::Instance();
}

TcpServer::~TcpServer()
{
}

void TcpServer::init()
{
    static const int SIZE = 1060;
    uint8_t* buffer = nullptr;
    int client, newsocketfd;
    size_t len;

    struct sockaddr_in serv_addr, client_addr;

    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Error opening socket!");
        exit(1);
    }

    memset((char*)&serv_addr, 0, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(1234);

    if (bind(socket_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("BIND failed");
        exit(1);
    }

    listen(socket_fd, 5);
    client = sizeof(client_addr);


    static char msg[128] = {0};
    // start buffer thread
    setName("tcp-thread");
    create(128 * 1024, 20, TcpServer::worker, this);
    for(;;) {

        newsocketfd = accept(socket_fd, (struct sockaddr*)&client_addr, (socklen_t*)&client);
        if (newsocketfd < 0) {
            perror("Error on accept");
            exit(1);
        }

        for(;;) {
            for(buffer = (uint8_t*)&m_frames, len = 0; len < SIZE;) {
                int nn = read(newsocketfd, buffer, SIZE - len);
                len += nn;
                buffer += nn;
            }

            m_lock.lock();
            m_buffer.enqueue(m_frames);
            m_lock.unlock();
#if 0
            for(int i=0; i < 16; ++i) {
                printf("%lu\n", (long unsigned int) m_frames[i].counter);
                if (++p_server->m_conn_info.paketCounter != m_frames[i].counter) {
                    p_server->m_conn_info.paketCounter = m_frames[i].counter;
                    snprintf(msg, sizeof(msg), "Missed: %lu\n",
                             (long unsigned int) p_server->m_conn_info.paketCounter+1);
                    utils::IPC::Instance().sendMessage(msg);
                } else {
                    //m_buffer.write(frame);
            }
#endif
        }

        close(newsocketfd);
    }

    close(socket_fd);
}

void TcpServer::deinit()
{
    //join();
    utils::IPC::Instance().sendMessage("TCP server deinit\n");
}

} // udp

} // plugin
