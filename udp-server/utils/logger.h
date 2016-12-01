#ifndef LOGGER_H
#define LOGGER_H

// component //
#include "writer.h"

namespace iz {

class Logger
{
public:
    static Logger& Instance();
    bool init();
    void deinit();
    void logMessage(const char *module, const QByteArray& msg);

private:
    explicit Logger();
    virtual ~Logger();

    Writer m_writer;
    static Logger* s_inst;
};


} // iz


#endif // LOGGER_H
