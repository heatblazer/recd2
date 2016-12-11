#include "utils/json-writer.h"

//static const char* THIS_FILE = "json-writer.cpp";

namespace iz {

JsonWriter::JsonWriter(QObject *parent)
    : QObject(parent)
{
}

JsonWriter::~JsonWriter()
{
}

void JsonWriter::init()
{
    // configure outside json filena for now use default
    static const char* fname = "metainfo.json";
                      // filename, initial buffsize, log speed //
    if (m_writer.setup(QString(fname), 1000, 500)) {
        m_writer.setObjectName("json writer");
        m_writer.startWriter();
    }
}

void JsonWriter::deinit()
{
    if (m_json.count() > 0) {
        m_writer.write(m_json);
    }
    m_writer.stopWriter();
}

void JsonWriter::write()
{
    m_writer.write(m_json);
    m_json.clear(); // make sure it`s cleared for new writing
}

JsonWriter &JsonWriter::add(const QString &tag)
{
    m_json.append(tag);
    return *this;
}


} // namespace iz
