// std //
#include <iostream>

// lib //
#include "utils.h"

// local headers //
#include "types.h"
#include "sapplication.h"
#include "unix/daemon.h"

// help message //
static const char* help_message = "This is a recording server over udp streams.\n"
        "The program is in early test mode and shall be used until proved"
        "and comes with no warranty!\n";

// the standart server test - passed for now
// I got a correct sine wave from the client
// and recorded it to a wav file
/// for now one option for
/// daemonizing
/// \brief getOpts
/// \param str
/// \return
///
static int getOpts(char* str)
{
    int opts = 0;
    if ( (strcmp(str, "-d") == 0) ||
         (strcmp(str, "--daemon") == 0)) {
        opts = 1;
    } else if( (strcmp(str, "-h") == 0) ||
               (strcmp(str, "--help")) == 0) {
        opts = 2;
    } else if ( (strcmp(str, "-c")) == 0 ||
                (strcmp(str, "--config")) == 0) {
        // reserved for future preloading of configuations!!!
        opts = 0;
    } else {
        opts = 3;
    }
    return opts;
}

int main(int argc, char *argv[])
{
    // if no args - run in lab mode to test
    // and debug , later we`ll be sure
    // the >1 arg is provided
    std::cout << "Starting recording server..."
              << std::endl;

    bool is_daemon = false;
    if (argc > 1) {
        int opts = 0;
        for(int i=0; i < argc; ++i) {
           opts = getOpts(argv[i]);
           switch(opts) {
           case 1: // and only for now
               std::cout << "Setting the server as daemon." << std::endl;
               is_daemon = true;
               break;
           case 2:
               std::cout << help_message;
               break;
           case 0:
               std::cout << "Stub!!! Preloading configuration!" << std::endl;
               break;
           case 3:
           default:
               std::cout << "No arguments!" << std::endl;
               break;
           }
        }
    } else {
        std::cout << "Usage:\n"
                  << "Load config: recd2 -c <path to conf file>\n"
                  << "Load config: recd2 --config <path to conf file>\n"
                  << "Print help and exit: recd2 -h\n"
                  << "Print help and exit: recd2 --help\n"
                  << "Daemonize: recd2 -d\n"
                  << std::endl;
        std::cout << "You will be entering a failsafe mode with defaults."
                  << std::endl;
    }

    if (is_daemon) {
        iz::Daemon::daemonize();
    }

    iz::SApplication app(argc, argv);
    int res = app.init();
    if (res < 0) {
        std::cout << "Error!\n"
                  << "Program failed to initalize with error: ("
                  << res << ") " << std::endl;
        exit(4);
    }

    return app.exec();
}
