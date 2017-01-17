#include "plugin-iface.h"
#include "qcapdev.h"

#include <QCoreApplication>

// some testing here...
int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    const interface_t* iface = get_interface();
    iface->main_proxy(argc, argv);
    iface->init();

    return app.exec();
}
