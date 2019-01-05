#ifndef THREAD_H
#define THREAD_H

#ifdef __UNIX

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

    /// this will be used for the realtime
    /// critical section
    /// \brief The SpinLock class
    ///
    class SpinLock
    {
    public:
        SpinLock();
        ~SpinLock();
        bool tryLock();
        bool unlock();
    private:
        // replaced volatile
        union {
            unsigned long int threadId;
        } m_atomicVar;
        unsigned int m_lckCount;
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
        bool locked() const;
    private:
        pthread_mutex_t m_mutex;
        pthread_mutexattr_t m_sched_param;
        bool is_locked;
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
         virtual int createThread(size_t stack_size, int priority, entryPoint cb, void* user_data);
         virtual void join();
         virtual void setThreadName(const char* name);
         virtual void suspend(unsigned long msec);
         bool compareTo(const pthread_t* const th);
         virtual void closeThread();

         // for the inheritance
    protected:
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
        void Lock();
        void Unlock();
    public:
        explicit BSemaphore();
        ~BSemaphore();
        void post();
        void wait();

    private:
        pthread_mutex_t m_lock;
        pthread_cond_t m_cond; // 0 or 1
        int m_count;
    };
}

#endif

#endif // THREAD_H

