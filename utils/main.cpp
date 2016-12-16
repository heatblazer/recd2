#include <QCoreApplication>
#include "logger.h"
#include "qwave-writer.h"
#include "writer.h"
#include "wav-writer.h"
#include "ipc-msg.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    utils::IPC::Instance().sendMessage("sjdsdsijdsidijsd");

    return a.exec();
}
