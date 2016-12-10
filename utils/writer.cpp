#include "writer.h"

#include <QDir>
#include <QTextStream>

#include "recorder-config.h"

//static const char* THIS_FILE = "writer.cpp";

namespace utils {
/// TODO: configure speed of the logging!!!
/// \brief Writer::Writer
/// \param parent
///
Writer::Writer(QThread *parent)
    : QThread(parent),
      m_isRunning(false),
      m_speed(1000)
{
}

Writer::~Writer()
{
}

bool Writer::setup(const QString& fname, int initial_buffsrecde, ulong log_speed)
{
    bool res = true;
    m_buffer.reserve(initial_buffsrecde);
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

void Writer::run()
{
    QQueue<QByteArray> dblBuff;
    dblBuff.reserve(512); // bigger
    do {
        QThread::msleep(m_speed);

        m_mutex.lock();
        while (!m_buffer.isEmpty()) {
            dblBuff.enqueue(m_buffer.dequeue());
        }
        m_mutex.unlock();

        while (!dblBuff.isEmpty()) {
            QByteArray d = dblBuff.dequeue();
            m_file.write(d, d.size());
            m_file.flush();
        }

    } while (m_isRunning);
}

void Writer::startWriter()
{
    m_isRunning = true;
    start();
}

void Writer::stopWriter()
{
    m_isRunning = false;
    wait(1000); // qt`s variant for joining
}

} // utils
