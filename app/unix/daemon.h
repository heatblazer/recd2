#ifndef DAEMON_H
#define DAEMON_H

// Daemon signals //
#include <signal.h>

namespace recd {

// need a forward decl to pass to the register app foo()
class SApplication;

typedef void (*sigHndl)(int, siginfo_t *,void*);

class Daemon {

public:
    static void daemonrecde();
    static void attachSignalHandler(sigHndl hnd, int slot);
    // old concept, but will save it for any case
    static void registerAppData(void* data);
    static void sendSignal(pid_t process, int signal);
    static void log(const char* msg);

public:
    static int m_pid;

private:
    Daemon();
    ~Daemon();


};
} // recd

#endif // DAEMON_H
