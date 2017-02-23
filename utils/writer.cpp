#include "writer.h"

#include <QDir>
#include <QTextStream>

#include "recorder-config.h"

//static const char* THIS_FILE = "writer.cpp";

namespace utils {

void *Writer::worker(void *pArgs)
{
    Writer* w = (Writer*) pArgs;

    QQueue<QByteArray> dblBuff;
    dblBuff.reserve(512); // bigger
    do {
        w->m_thread.suspend(w->m_speed);
        w->m_mutex.lock();
        while (!w->m_buffer.isEmpty()) {
            dblBuff.enqueue(w->m_buffer.dequeue());
        }
        w->m_mutex.unlock();

        while (!dblBuff.isEmpty()) {
            QByteArray d = dblBuff.dequeue();
            w->m_file.write(d, d.size());
            w->m_file.flush();
        }
    } while (w->m_isRunning);

    return (int*)0;
}



/// TODO: configure speed of the logging!!!
/// \brief Writer::Writer
/// \param parent
///
Writer::Writer()
    : m_isRunning(false),
      m_speed(1000)
{
    m_mutex.init();
}

Writer::~Writer()
{
    // mebers will be destroyed automatically
}

bool Writer::setup(const QString &fname, int initial_buffsize, ulong log_speed)
{
    bool res = true;
    m_buffer.reserve(initial_buffsize);
    m_speed = log_speed;
    if (!m_file.isOpen()) {
        m_file.setFileName(fname);
        m_file.open(QIODevice::Append|QIODevice::ReadWrite);
    }
    return res;
}

void Writer::write(const QByteArray &data)
{
    m_mutex.lock();
    m_buffer.append(data);
    m_mutex.unlock();
}

void Writer::startWriter()
{
    m_isRunning = true;
    m_thread.createThread(64 * 1024, 10, Writer::worker, this);
}

void Writer::stopWriter()
{
    m_isRunning = false;
    m_thread.join();
    m_thread.closeThread();
}

void Writer::setObjectName(const QString &name)
{
    m_thread.setThreadName(name.toStdString().data());
}

} // utils
