//////////// THIS IS A UNIT LIB TEST //////////////

#include <iostream>
#include "plugin-interface.h"

using namespace std;

int main(int argc, char *argv[])
{
    const interface_t* dft = get_interface();
    dft->init();
    dft->main_proxy(argc, argv);
    dft->deinit();

    return 0;
}

