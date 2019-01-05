#ifndef WRITER_H
#define WRITER_H

#include <QFile>
#include <QThread>
#include <QMutex>
#include <QQueue>

#include "thread.h"

namespace utils {

class Writer : public QThread
{
public:

    static void* worker(void* pArgs);
    Writer();
    virtual ~Writer();
    // we may need to extend this class later
    bool setup(const QString &fname, int initial_buffsize, ulong log_speed);
    void write(const QByteArray& data);
    void startWriter();
    void stopWriter();

protected:
    virtual void run();

private:

    QFile               m_file;
    QQueue<QByteArray>  m_buffer;
    bool                m_isRunning;
    ulong               m_speed;
    QMutex              m_mutex;
};

} // utils
#endif // WRITER_H
