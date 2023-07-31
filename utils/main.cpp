// some test main...
#include <QCoreApplication>
#include "ipc-msg.h"
#include "ring-buffer.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    utils::IPC::Instance().sendMessage("111", "sjdsdsijdsidijsd");
    // test the ring buffer
    utils::RingBuffer<frame_data_t, 512> rb;
    rb.init();
    frame_data_t d = {1, {0}, {0}};

    for(int i=0; i < 10; ++i) {
        rb.write(d);
    }



    return a.exec();
}
