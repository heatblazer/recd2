#ifndef LOGGER_H
#define LOGGER_H

// component //
#include "writer.h"

namespace utils {
class Logger
{
public:
    static Logger& Instance();
    bool init();
    void deinit();
    void logMessage(const char *module, const QByteArray& msg);
    void setWriterName(const QString& name);
private:
    explicit Logger();
    virtual ~Logger();

    Writer m_writer;
    bool m_onetime;
    static Logger* s_inst;
};

} // utils

#endif // LOGGER_H
