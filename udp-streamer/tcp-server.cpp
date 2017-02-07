#include "tcp-server.h"

#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>

#include "server.h"
#include "utils.h"


namespace plugin {
namespace udp {


void *TcpServer::worker(void *pArgs)
{

    TcpServer* s = (TcpServer*) pArgs;
    QQueue<udp_data_t> dblBuff;

    for(;;) {

        s->m_lock.lock();
        while (!s->m_buffer.empty()) {
            dblBuff.enqueue(s->m_buffer.dequeue());
        }
        s->m_lock.unlock();

        while (!dblBuff.empty()) {
            udp_data_t frame = dblBuff.dequeue();
            QList<sample_data_t> ls;
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
            // finally send it
            s->p_server->put_data((QList<sample_data_t>*) &ls);
        }
        s->suspend(1);
    }

}

TcpServer::TcpServer()
    : p_server(nullptr),
      socket_fd(-1)
{
}

TcpServer::~TcpServer()
{
}

void TcpServer::init()
{
    Server* s = &Server::Instance();
    p_server = s;

    uint8_t* buffer = nullptr;
    udp_data_t frame = {0, {0}, {{0}}};
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
    create(64 * 1024, 10,TcpServer::worker, this);
    for(;;) {

        newsocketfd = accept(socket_fd, (struct sockaddr*)&client_addr, (socklen_t*)&client);
        if (newsocketfd < 0) {
            perror("Error on accept");
            exit(1);
        }

        for(;;) {
            for(buffer = (uint8_t*)&frame, len = 0; len < sizeof(frame);) {
                int nn = read(newsocketfd, buffer, sizeof(frame) - len);
                len += nn;
                buffer += nn;
            }
            printf("%lu\n", (long unsigned int) frame.counter);

            if (++s->m_conn_info.paketCounter != frame.counter) {
                s->m_conn_info.paketCounter = frame.counter;
                snprintf(msg, sizeof(msg), "Missed: %lu\n",
                         (long unsigned int) s->m_conn_info.paketCounter+1);

                utils::IPC::Instance().sendMessage(msg);
            } else {
                m_lock.lock();
                m_buffer.enqueue(frame);
                m_lock.unlock();
            }
        }

        close(newsocketfd);
    }

    close(socket_fd);
}

void TcpServer::deinit()
{
    join();
    utils::IPC::Instance().sendMessage("TCP server deinit\n");
}


} // udp

} // plugin
