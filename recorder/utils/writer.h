#ifndef WRITER_H
#define WRITER_H

#include <QFile>
#include <QThread>
#include <QMutex>
#include <QQueue>

namespace plugin  {
namespace rec {


class Writer : public QThread
{
    Q_OBJECT
public:
    explicit Writer(QThread* parent=nullptr);
    virtual ~Writer();
    // we may need to extend this class later
    bool setup(const QString &fname, int initial_buffsize, ulong log_speed);
    void write(const QByteArray& data);
    virtual void run();
    void startWriter();
    void stopWriter();

private:
    QFile               m_file;
    QMutex              m_mutex;
    QQueue<QByteArray>  m_buffer;
    bool                m_isRunning;
    ulong               m_speed;

};

} // rec
} // plugin
#endif // WRITER_H
