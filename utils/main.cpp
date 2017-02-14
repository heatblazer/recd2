// some test main...
#include <QCoreApplication>
#include "logger.h"
#include "qwave-writer.h"
#include "writer.h"
#include "wav-writer.h"
#include "ipc-msg.h"
#include "ring-buffer.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    utils::IPC::Instance().sendMessage("sjdsdsijdsidijsd");
    // test the ring buffer
    utils::RingBuffer rb;
    rb.init();
    utils::udp_data_t d = {1, {0}, {{0}}};

    for(int i=0; i < 10; ++i) {
        rb.write(d);
    }



    return a.exec();
}
