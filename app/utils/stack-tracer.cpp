#include "stack-tracer.h"

// ansi C //
#include <stdio.h> // prinf to stderr

namespace recd {

bool        StackTrace::m_isOnetime = false;
StackTrace* StackTrace::s_inst = nullptr;

char        StackTrace::m_stack[StackTrace::SIZE] = {0};
stack_t     StackTrace::sigsegStack;
struct sigaction StackTrace::s_sigHandler;
const char* StackTrace::p_filename = nullptr;


StackTrace::StackTrace(const char *fname)
{
    if (fname != nullptr) {
        p_filename = fname;
    }
}


StackTrace::~StackTrace()
{
}


StackTrace& StackTrace::Instance()
{
    if (s_inst == nullptr) {
        s_inst = new StackTrace("/tmp/radis-ui-crash.log");
    }
    return *s_inst;
}


///
/// Onetime init function for the tracer
/// \return true, should not fail
///
bool StackTrace::init()
{
    if (!m_isOnetime) {
        m_isOnetime = true;

        sigsegStack.ss_sp = m_stack;
        sigsegStack.ss_flags = SS_ONSTACK;
        sigsegStack.ss_size = sizeof(m_stack);

        // register the stack
        sigaltstack(&sigsegStack, nullptr);

        s_sigHandler.sa_handler = &StackTrace::signalHandler;
        s_sigHandler.sa_flags = SA_ONSTACK;

        // register the handler
        sigaction(SIGSEGV, &s_sigHandler, NULL);
    }

    return m_isOnetime;
}



/// SIGSEGV handler implementation
void StackTrace::signalHandler(int sig)
{
    void* buffer[10U] = {0}; // MISRA zeroinit 2d arrays
    char** strings = {0};

    int nptrs = backtrace(buffer, 10U);
    strings = backtrace_symbols(buffer, nptrs);
    if (p_filename != nullptr) {
        FILE* fp = fopen(p_filename, "a+");
        if (fp == NULL) {
            return ;
        }
        fprintf(fp, "Signal caught: [%d]\n", sig);

        fprintf(fp,"================BACKTRACE-BEGIN================\n");
        fprintf(fp,"StackTrace returned [%d] functions\n", nptrs);

        for (int i = 0; i < nptrs; ++i) {
            fprintf(fp, "%s\n", strings[i]);
        }
        fprintf(fp, "================BACKTRACE-END================\n");
        fclose(fp);
    } else {
        fprintf(stderr, "Signal caught: [%d]\n", sig);

        fprintf(stderr,"================BACKTRACE-BEGIN================\n");
        fprintf(stderr,"StackTrace returned [%d] functions\n", nptrs);

        for(int i = 0; i < nptrs; ++i) {
            fprintf(stderr, "%s\n", strings[i]);
        }
        fprintf(stderr, "================BACKTRACE-END================\n");
    }
}

} // namespace recd
