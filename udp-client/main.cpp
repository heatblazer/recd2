#include <QCoreApplication>
#include "client.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    iz::Client c;
    c.init();
    return a.exec();
}
