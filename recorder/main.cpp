#include <QCoreApplication>

#include "plugin-iface.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc,argv);
    const interface_t* iface = get_interface();
    iface->init();
    return app.exec();
}