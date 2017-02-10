#include "thread.h"

#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sched.h>

namespace utils {

PMutex::PMutex()
    : is_locked(false)
{
}

PMutex::~PMutex()
{
}

int PMutex::init()
{
    int ret = -1;
    // TODO: play with atrribs later
    //ret = pthread_mutexattr_init(&m_sched_param);
    //ret = pthread_mutexattr_setpshared(&m_sched_param, PTHREAD_PROCESS_SHARED);
    ret = pthread_mutex_init(&m_mutex, nullptr);
    return 0;
}

void PMutex::lock()
{
    is_locked = true;
    pthread_mutex_lock(&m_mutex);
}

void PMutex::unlock()
{
    is_locked = false;
    pthread_mutex_unlock(&m_mutex);
}

void PMutex::deinit()
{
    //pthread_mutexattr_destroy(&m_sched_param);
    pthread_mutex_destroy(&m_mutex);
}

const bool PMutex::locked()
{
    return is_locked;
}

pthread_t PThread::currentThread()
{
    pthread_t t = pthread_self();
    return t;
}

/// By default SCHED_RR - round robin,
/// give a time slice for every task to complete
/// a better FIFO sched, as suggested in linux man pages:
/// http://man7.org/linux/man-pages/man7/sched.7.html
/// \brief PThread::PThread
/// \param Scheduler algorithm: FIFO, RoundRobin(default),
///
PThread::PThread(SchedAlgorithms algo)
    : m_schedAlgo(algo),
      m_name{0},
      m_stack(nullptr)
{
}

PThread::~PThread()
{
}

/// this is quite unsafe and simple
/// I`ll rework it to match a comfort thread
/// api, with scheduler and attributes for now
/// it`s ok and works better than qthread.
/// \brief PThread::create
/// \param cb
/// \param user_data
///
int PThread::create(size_t stack_size, int priority, entryPoint cb, void* user_data)
{

    size_t default_size ;
    int ret = -1;
    ret = pthread_attr_init(&m_attr);

    // safe to get schedparam
    ret = pthread_attr_getschedparam(&m_attr, &m_sched_param);

    if (priority > 20) {
        priority = 20;
    } else if (priority < 0) {
        priority = 15;
    } else {
        // misra stuff
    }

    pthread_attr_setschedpolicy(&m_attr, m_schedAlgo);
    m_sched_param.__sched_priority = priority;

    ret = pthread_attr_setschedparam(&m_attr, &m_sched_param);

    pthread_attr_getstacksize(&m_attr, &default_size);

    if (stack_size >= default_size * 2) {
        stack_size = default_size;
    }

    if (stack_size < default_size) {
        stack_size = default_size;
    }

    // guaranted 8bit stack
    m_stack = new uint8_t[stack_size];
    memset(m_stack, 0, stack_size); // null memory

    ret = pthread_attr_setstack(&m_attr, m_stack, stack_size);

    // create thread
    ret = pthread_create(&m_thread, &m_attr, cb, user_data);

    if (strlen(m_name) <= 0) {
        // some default name
        snprintf(m_name, 64, "Thread-%d", (int)m_thread);
    }

    ret = pthread_setname_np(m_thread, m_name);
    return ret;

}

void PThread::join()
{
    void* ret;
    int* r = (int*) ret;
    pthread_join(m_thread, &ret);
}

void PThread::setName(const char *name)
{
    //pthread_setname_np(m_thread, name);
    strncpy(m_name, name, 64);
}

void PThread::suspend(unsigned long msec)
{
    usleep(msec);
}

/// Note that pthread_t is opaqe so even if
/// == is used the comparison will fail, so as POSIX suggest,
/// == shall not be used but the posix function:
/// pthread_equal(t1, t2)
/// compare this thread to another
/// 0 if they are not the same
/// 1 or greater if they are the same
/// \brief PThread::compareTo
/// \param another thread
/// \return true if are the same thread, false else
///
bool PThread::compareTo(const pthread_t * const th)
{
    int cmp = pthread_equal(m_thread, *th);
    return (cmp > 0);
}

void PThread::deinit()
{
    if (m_stack != nullptr) {
        delete [] m_stack;
        m_stack = nullptr;
    }
    pthread_attr_destroy(&m_attr);

}

BSemaphore::BSemaphore(const char *name)
{
    strncpy(m_name, name, sizeof(m_name)/sizeof(m_name[0]));
}

BSemaphore::~BSemaphore()
{

}

SpinLock::SpinLock()
    : m_lckCount(0)
{

}

SpinLock::~SpinLock()
{

}

bool SpinLock::tryLock()
{
    pthread_t t = PThread::currentThread();
    bool lock_succeded = (m_atomicVar.threadId == t) ||
            __sync_bool_compare_and_swap(&m_atomicVar.threadId, 0, t);
    if (lock_succeded) {
        m_lckCount++;
    }

    return lock_succeded;
}


bool SpinLock::unlock()
{
    pthread_t t = PThread::currentThread();
    bool unlock_success = false;

    if (m_atomicVar.threadId == t) {
        m_lckCount--;
        unlock_success = (m_lckCount != 0) ||
                __sync_bool_compare_and_swap(&m_atomicVar.threadId, t, 0);
    }

    return unlock_success;
}


} // utils
