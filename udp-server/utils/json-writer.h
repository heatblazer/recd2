#ifndef JSONWRITER_H
#define JSONWRITER_H

// qt sutff //
#include <QObject>
#include <QHash>
#include <QList>
#include <QPair>

// threaded writer //
#include "writer.h"

namespace iz {

class JsonWriter : public QObject
{
    Q_OBJECT
public:
    explicit JsonWriter(QObject* parent=nullptr);
    virtual ~JsonWriter();
    void init();
    void deinit();
    void write();
    JsonWriter& add(const QString& tag);

private:
    Writer          m_writer;
    QByteArray      m_json;

};
} // iz

#endif // JSONWRITER_H
