#include <QCoreApplication>
#include "logger.h"
#include "qwave-writer.h"
#include "writer.h"
#include "wav-writer.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    utils::Logger::Instance().logMessage("main.cpp", "bla bla bla");

    return a.exec();
}
