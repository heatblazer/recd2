#ifndef STACKTRACER_H
#define STACKTRACER_H
// system V specific //
#include <unistd.h>

// backtrace //
#include <execinfo.h>

// posix signals //
#include <signal.h>


namespace recd {

class StackTrace
{
public:
    static StackTrace& Instance();

    // one instance of this class, one shot init, never
    // called anywhere so I`ll guard the init function
    static bool init();

private:
    explicit StackTrace(const char* fname = nullptr);
    ~StackTrace();

    // for future this might be changed as a function pointer
    // to have multiple signal handlers , for now it`s
    // just a tracer
    static void signalHandler(int sig);

private:
    enum StackSize { SIZE = 64 * 1024 }; // enough stack for the signal stack

    static bool             m_isOnetime;
    static StackTrace*      s_inst;

    // stack for the traceback program
    static char             m_stack[StackSize::SIZE];

    static stack_t          sigsegStack;
    static struct sigaction s_sigHandler;
    static const char*      p_filename;
};

} // recd

#endif // STACKTRACER_H
