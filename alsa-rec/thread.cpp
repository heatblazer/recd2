#include "thread.h"

#include <unistd.h>
#include <string.h>


#define DEFAULT_STACK_SIZE (64 * 1024)

namespace plugin {
namespace alsarec {

PThread::PThread()
{
    memset(m_name, 0, sizeof(m_name)/sizeof(m_name[0]) );
}

PThread::~PThread()
{

}

bool PThread::create(int stack_size, void *usr_data, entryCb callback, int prio)
{
    (void) prio; // unimplemented schedparams for now...

    bool ret = false;
    int result = -1;

    if (stack_size < DEFAULT_STACK_SIZE) {
        stack_size = DEFAULT_STACK_SIZE;
    }

    result = pthread_create(&m_thread, NULL, callback, usr_data);
    if (result == 0) {
        ret = true;
        m_running = true;
    }

    return ret;
}

/// joins the thread
/// \brief PThread::join
///
void PThread::join()
{
    pthread_join(m_thread, NULL);
    m_running = false;
}

void PThread::setName(const char *name)
{
    strcpy(m_name, name);
    pthread_setname_np(m_thread, name);
}

/// shadws unsitd sleep :(
/// \brief PThread::sleep
/// \param msec
///
void PThread::sleep(unsigned long msec)
{
    usleep(msec * 1000);
}

///
/// \brief PThread::currentThread
/// \return current running thread
///
pthread_t PThread::currentThread()
{
    return pthread_self();
}

bool PThread::isRunning() const
{
    return m_running;
}

PMutex::PMutex()
{

}

PMutex::~PMutex()
{

}

void PMutex::init()
{
    pthread_mutex_init(&m_mutex, NULL);
}

void PMutex::lock()
{
    pthread_mutex_lock(&m_mutex);
}

void PMutex::unlock()
{
    pthread_mutex_unlock(&m_mutex);
}

void PMutex::destroy()
{
    pthread_mutex_destroy(&m_mutex);
}


} // alsarec
} // plugin
