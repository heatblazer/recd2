#include "logger.h"

#include <QDir>

#include "recorder-config.h"
#include "date-time.h"

namespace utils {
Logger* Logger::s_inst = nullptr;

Logger::Logger()
    : m_onetime(false)
{
}

Logger::~Logger()
{
}

Logger &Logger::Instance()
{
    if (s_inst == nullptr) {
        s_inst = new Logger;
    }
    return *s_inst;
}

bool Logger::init()
{
    if (!m_onetime) {
        m_onetime = true;
        bool res = true;

        const MPair<QString, QString> log_dir = RecorderConfig::Instance()
                .getAttribPairFromTag("Paths", "logs");

        const MPair<QString, QString> name = RecorderConfig::Instance()
                .getAttribPairFromTag("Log", "name");

        const MPair<QString, QString> time = RecorderConfig::Instance()
                .getAttribPairFromTag("Log", "timestamp");

        const MPair<QString, QString> speed = RecorderConfig::Instance()
                .getAttribPairFromTag("Log", "speed");

        char fname[512] = {0};

        if (log_dir.m_type1 != "") {
            if (!QDir(log_dir.m_type2).exists()) {
                QDir().mkdir(log_dir.m_type2);
                snprintf(fname, sizeof(fname), "%s/", log_dir.m_type2.toStdString().data());
            } else {
                snprintf(fname, sizeof(fname), "%s/", log_dir.m_type2.toStdString().data());
            }
        }

        // setup logger logging speed
        ulong log_speed = 0;
        if (speed.m_type1 != "") {
            bool res = false;
            log_speed = speed.m_type2.toULong(&res);
            if (!res) {
                log_speed = 1000;
            }
        }

        // setup timestamp
        if (time.m_type1 != "") {
            if (time.m_type2 == "enabled" || time.m_type2 == "true") {
                strcat(fname, DateTime::getDateTime());
                strcat(fname, "-");
            }
        }

        // setup filename
        if (name.m_type1 != "") {
            if (name.m_type2 != "") {
                strcat(fname, name.m_type2.toStdString().data());
            }
        }

        if (m_writer.setup(QString(fname), 100, log_speed)) {
            m_writer.setObjectName("writer thread");
            m_writer.startWriter();
        }

        return res;
    } else {
        return m_onetime;
    }

}

void Logger::deinit()
{
    m_writer.stopWriter();
}

/// implement the module
/// \brief Logger::logMessage
/// \param modlue - caller
/// \param msg
///
void Logger::logMessage(const char* module, const QByteArray &msg)
{
    (void) module;
    m_writer.write(msg);
}

void Logger::setWriterName(const QString &name)
{
    m_writer.setObjectName(name);
}

} // utils
