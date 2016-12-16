#include "plugin-interface.h"
#include <QCoreApplication>

/// main to test when unit tests
/// \brief main
/// \param argc
/// \param argv
/// \return
///
int main(int argc, char *argv[])
{
    QCoreApplication app(argc,argv);
    const interface_t* iface = get_interface();
    iface->init();
    return app.exec();
}
