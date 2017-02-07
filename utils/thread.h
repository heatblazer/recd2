#ifndef THREAD_H
#define THREAD_H

#include <pthread.h>
#include <stdint.h>

/// И.Z.`s threading API based on POSIX:
/// The main reason for this pthread_t implementation API
/// is that the QThread api even tho it`s OS independent, somehow,
/// does not gives me the flexibility of using the raw thread API for
/// linux, however, this will totally stops the program being tested on
/// Windows machines, but I`ll got more power and felxibility with the
/// posix api.
/// Some may advise using std::thread/std::mutex however I still prefer the
/// POSIX eclsuisve api only. If you don`t like my PThread impelmentation, jsut
/// revert to QThread or std::thread, as you like.
/// Also I can set thread priorities as desired.

namespace utils {
    class PThread;
    class PMutex;
    class BSemaphore;

    typedef void* (*entryPoint)(void*);

    class SpinLock
    {
    public:
        SpinLock();
        ~SpinLock();
        void lock();
        void unlock();
    private:
        volatile int m_lock;
    };

    class PMutex
    {
    public:
        PMutex();
        ~PMutex();
        int init();
        void lock();
        void unlock();
        void deinit();
    private:
        pthread_mutex_t m_mutex;
        pthread_mutexattr_t m_sched_param;
    };

    class PThread
    {
    public:
        enum SchedAlgorithms
        {
            SchedOther = SCHED_OTHER,
            SchedFifo = SCHED_FIFO,
            SchedRR = SCHED_RR,
            #ifdef __USE_GNU
            SchedBatch = SCHED_BATCH,
            SchedIdle = SCHED_IDLE,
            SchedResetOnFork = SCHED_RESET_ON_FORK
            #endif
        };

    public:
        static pthread_t currentThread();
         explicit PThread(SchedAlgorithms algo = SchedRR);
        ~PThread();
         int create(size_t stack_size, int priority, entryPoint cb, void* user_data);
         void join();
         void setName(const char* name);
         void suspend(unsigned long msec);
         bool compareTo(const pthread_t* const th);
         void deinit();

    private:
         SchedAlgorithms    m_schedAlgo;
         pthread_t m_thread;        // pthread
         pthread_attr_t m_attr;     // ptrhead attributes
         sched_param m_sched_param; // scheduler param used for set priority
         uint8_t* m_stack;
         char m_name[64];
    };

    // binary semaphore using mutexes conditionals and counters
    class BSemaphore
    {
        // TODO: to be implemented
    public:
        explicit BSemaphore(const char* name);
        ~BSemaphore();
        int create();
        void post();
        void wait();
        void close();
    private:
        PMutex m_lock;
        pthread_cond_t m_nonzero;
        unsigned m_count;
        char m_name[64];
    };
}
#endif // THREAD_H
