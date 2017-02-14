#include "tcp-server.h"

#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h> // error handlig from the socket fd based on timeout

#include "server.h"
#include "utils.h"

#define READ_BUFF_MULTIPLIER(A) ((A) * (sizeof(utils::udp_data_t)))

namespace plugin {
namespace udp {


void *TcpServer::worker(void *pArgs)
{
    TcpServer* s = (TcpServer*) pArgs;

    static const int SIZE = 1060;
    uint8_t* buffer = nullptr;
    int client, newsocketfd;
    size_t len;

    int select_result = 0;
    fd_set read_set;
    struct timeval timeout;

    timeout.tv_sec = 20;
    timeout.tv_usec = 0;

    FD_ZERO(&read_set);

    static char msg[128] = {0};
    struct sockaddr_in serv_addr, client_addr;


    if ((s->socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Error opening socket!");
        exit(1);
    }

    FD_SET(s->socket_fd, &read_set);

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

    for(;;) {
        select_result = select(s->socket_fd+1, &read_set, NULL, NULL, &timeout);
        if (select_result < 0) {
            // handle error
            utils::IPC::Instance().sendMessage("Error in select()\n");
            break;

        } else if (select_result == 0) {
            // handle timeout
            utils::IPC::Instance().sendMessage("select handle timed out after 20 seconds\n");
            break;
        } else {
            newsocketfd = accept(s->socket_fd, (struct sockaddr*)&client_addr, (socklen_t*)&client);
            if (newsocketfd < 0) {
                perror("Error on accept");
                exit(1);
            }

            for(;;) {
                utils::udp_data_t frame = {0, {0}, {{0}}};
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
//                    s->m_buffer.isBusy = true;
                    s->m_lock.lock();
                    s->m_buffer.data.write(frame);
                    s->m_lock.unlock();
//                    s->m_buffer.isBusy = false;
                }
            }
            // close old conn - register a new one
            close(newsocketfd);
        }
    }

    utils::IPC::Instance().sendMessage("Stoppint TCP server...\n");
    // finally close socket
    close(s->socket_fd);
}

TcpServer::TcpServer()
    :
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
    create(128 * 1024, 20, TcpServer::worker, this);
    m_writer->init();
}

void TcpServer::deinit()
{
    utils::IPC::Instance().sendMessage("TCP server deinit\n");
    utils::PThread::join();
    m_lock.deinit();
    m_writer->join();
}

TcpServer::Writer::Writer(TcpServer * const p)
    : ref(p)
{
}

void *TcpServer::Writer::worker(void *pArgs)
{
    Writer* w  = (Writer*) pArgs;

    for (;;) {
        QList<utils::sample_data_t> ls;

        utils::udp_data_t* readData = nullptr;
        int read_size = 0;

        w->lock.lock();
        read_size = w->ref->m_buffer.data.readAll(&readData);
        w->lock.unlock();

        for (int i=0; i < read_size; ++i) {
            std::cout << "Read size: " << read_size << std::endl;
            std::cout << readData[i].counter << std::endl;
            for(int j=0; j < 32; ++j) {
                utils::sample_data_t smpls = {0, 0};
                short smpl[16] = {0};
                smpls.samples = smpl;
                smpls.size = 16;
                for(int h=0; h < 16; ++h) {
                    smpls.samples[j] = readData[i].data[j][h];
                }
                ls.append(smpls);
            }
        }

        Server::Instance().put_data((QList<utils::sample_data_t>*)&ls);
        delete [] readData;
    }
}

void TcpServer::Writer::init()
{
    lock.init();
    setName("tcp-writer-thread");
    create(128 * 1024, 10, Writer::worker, this);
}


} // udp

} // plugin
