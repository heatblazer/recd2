#ifndef JSONWRITER_H
#define JSONWRITER_H

// qt sutff //
#include <QObject>
#include <QHash>
#include <QList>
#include <QPair>

// lib //
#include "utils.h"

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
    utils::Writer   m_writer;
    QByteArray      m_json;

};
} // iz

#endif // JSONWRITER_H
