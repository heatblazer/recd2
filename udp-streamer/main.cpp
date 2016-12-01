#include "plugin-interface.h"
#include <QCoreApplication>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc,argv);
    interface_t* iface = get_interface();
    iface->init();
    return app.exec();
}
