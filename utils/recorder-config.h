#ifndef RECORDERCONFIG_H
#define RECORDERCONFIG_H

#include <QObject>
#include <QHash>
#include <QFile>

#include "types.h"


namespace utils {

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

} // utils

#endif // RECORDERCONFIG_H
