#ifndef RECORDERCONFIG_H
#define RECORDERCONFIG_H

#include <QFile>
#include <QObject>
#include <QHash>

#include "types.h"

namespace iz {

// C++ "using" keyword to replace typedef
using PairList = QList<MPair<QString, QString> > ;

class RecorderConfig : public QObject
{

public:
    static RecorderConfig& Instance();
    bool loadFile(const QString& fname);
    bool fastLoadFile(const QString& fname);
    bool loadDefaults();
    PairList& getTagPairs(const QString& tag);
    const MPair<QString, QString> &getAttribPairFromTag(const QString &tag, const QString& attrib);
private:
    explicit RecorderConfig(QObject* parent=nullptr);
    ~RecorderConfig();

    static RecorderConfig* s_inst;
    QString m_filename;
    // tag name, attributes
    QHash<QString, QList<MPair<QString, QString> > > m_tags;
};

}  // iz

#endif // RECORDERCONFIG_H
