#include "alsarec.h"
#include "plugin-iface.h"

int main(int argc, char *argv[])
{

    const interface_t* iface = get_interface();

    iface->main_proxy(argc, argv);
    iface->init();
    while(1);
    return 0;
}
