#ifndef PTHREAD_H
#define PTHREAD_H

// posix threads //
#include <pthread.h>

typedef void* (*entryCb)(void*);

namespace plugin {
    namespace alsarec {

    class PThread
    {
    public:
        PThread();
        ~PThread();

        bool create(int stack_srecde, void* usr_data, entryCb callback, int prio);
        void join();
        void setName(const char* name);
        void sleep(unsigned long msec);
        void yield();
        static pthread_t currentThread();
        void setRunning(bool tf);
        bool isRunning() const;

    private:
        pthread_t m_thread;
        char m_name[128];
        bool m_running;
    };



    class PMutex
    {
    public:
        PMutex();
        ~PMutex();

        void init();
        void lock();
        void unlock();
        void destroy();
    private:
        pthread_mutex_t m_mutex;

    };


    template <typename T> class LockGuard
    {
    public:
        explicit LockGuard(const T& ref)
            : m_mutex(ref)
        {
            m_mutex.lock();
        }
        ~LockGuard()
        {
            m_mutex.unlock();
        }

    private:
        T& m_mutex;
    };

    } // alsarec
} // plugin

#endif // PTHREAD_H
