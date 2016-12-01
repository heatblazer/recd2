#include "recorder-config.h"
// remove me
#include <iostream>

#include <QXmlStreamReader>

static const char* THIS_FILE = "recorder-config.cpp";

namespace plugin {
namespace rec {


RecorderConfig* RecorderConfig::s_inst = nullptr;

RecorderConfig::RecorderConfig(QObject *parent)
    : QObject(parent)
{
}

RecorderConfig::~RecorderConfig()
{
}

/// TODO: update it!!!
/// this has to be updated for more defensive
/// programming and parsing, for now I am using
/// fastLoadFile(...) that does no checks so I am
/// doing this in the object requesting it, but next
/// I`ll make sure all is OK from this point.
/// \brief RecorderConfig::loadFile
/// \param fname
/// \return
///
bool RecorderConfig::loadFile(const QString &fname)
{
    bool res = false;
    QFile file(fname);
    if (file.open(QIODevice::ReadOnly)) {
        QXmlStreamReader reader(file.readAll());
        file.close();

        if (reader.hasError()) {
            res = false;
        } else {
            res = true;
            while(!reader.atEnd()) {
                reader.readNext();
                if (reader.isStartElement()) {
                    // rework the logic here
                    if (reader.name() == "HotSwap") {
                        QXmlStreamAttributes attribs = reader.attributes();
                        for(int i=0; i < attribs.count(); ++i) {
                            m_tags["HotSwap"].append(MPair<QString, QString>(
                                                         attribs.at(i).name().toString(),
                                                          attribs.at(i).value().toString()));
                        }
                    } else if (reader.name() == "Channels") {
                        QXmlStreamAttributes attribs = reader.attributes();
                        for(int i=0; i < attribs.count(); ++i) {
                            m_tags["Channels"].append(MPair<QString, QString>
                                                      (attribs.at(i).name().toString(),
                                                           attribs.at(i).value().toString()));
                        }

                    } else if (reader.name() == "Wave") {
                        QXmlStreamAttributes attribs = reader.attributes();
                        for(int i=0; i < attribs.count(); ++i) {
                            m_tags["Wave"].append(MPair<QString, QString>
                                                  (attribs.at(i).name().toString(),
                                                           attribs.at(i).value().toString()));

                        }

                    } else if (reader.name() == "Record"){
                        QXmlStreamAttributes attribs = reader.attributes();
                        for(int i=0; i < attribs.count(); ++i) {
                            m_tags["Record"].append(MPair<QString, QString>
                                                    (attribs.at(i).name().toString(),
                                                           attribs.at(i).value().toString()));
                        }
                    } else if (reader.name() == "Network"){
                        QXmlStreamAttributes attribs = reader.attributes();
                        for(int i=0; i < attribs.count(); ++i) {
                            m_tags["Network"].append(MPair<QString, QString>
                                                    (attribs.at(i).name().toString(),
                                                           attribs.at(i).value().toString()));
                        }
                    } else if (reader.name() == "Log") {
                        QXmlStreamAttributes attribs = reader.attributes();
                        for(int i=0; i < attribs.count(); ++i) {
                            m_tags["Log"].append(MPair<QString, QString>
                                                    (attribs.at(i).name().toString(),
                                                           attribs.at(i).value().toString()));
                        }
                    } else if (reader.name() == "Heartbeat") {
                        QXmlStreamAttributes attribs = reader.attributes();
                        for(int i=0; i < attribs.count(); ++i) {
                            m_tags["Heartbeat"].append(MPair<QString, QString>
                                                    (attribs.at(i).name().toString(),
                                                           attribs.at(i).value().toString()));
                        }
                    } else if (reader.name() == "Plugin") {


                    } else {
                        // dummy else for now
                        // later will support
                        // default config
                    }
                }
            }
        }
    }
    // safety check later !!!
    return res;
}

/// this is fast, non precision, no checking
/// function to just load all tags and attribs
/// I`ll use the safe version, for now it`s easier
/// to just load all stuff from the XML file then
/// work with it...
/// \brief RecorderConfig::fastLoadFile
/// \param fname
/// \return always true since it`s UNSAFE
bool RecorderConfig::fastLoadFile(const QString &fname)
{
    bool res = false;
    if (fname == "") {
        return res;
    }
    QFile file(fname);
    if (file.open(QIODevice::ReadOnly)) {
        QXmlStreamReader reader(file.readAll());
        file.close();
        while(!reader.atEnd()) {
            reader.readNext();
            if (reader.isStartElement()) {
                QXmlStreamAttributes attribs = reader.attributes();
                for(int i=0; i < attribs.count(); ++i) {
                    m_tags[reader.name().toString()]
                            .append(MPair<QString, QString>
                                              (attribs.at(i).name().toString(),
                                               attribs.at(i).value().toString()));
                }
            }
        }
        res = true;
    } else {
        res = false;
    }
    return res;
}

/// load dfault settings, this will be used
/// in case no config is present, I`ll setup
/// a default list of attribs to provide a default
/// behaviour.
/// \brief RecorderConfig::loadDefaults
/// \return true always
///
bool RecorderConfig::loadDefaults()
{
    bool res = true;

    // channels count - default 32
    m_tags["Channels"].append(MPair<QString, QString>(QString("count"),
                                                    QString("32")));

    // hotswap defaults - time based each 30 minutes
    m_tags["HotSwap"].append(MPair<QString, QString>(QString("timeBased"),
                             QString("enabled")));
    m_tags["HotSwap"].append(MPair<QString, QString>(QString("maxSize"),
                             QString("100MB")));
    m_tags["HotSwap"].append(MPair<QString, QString>(QString("interval"),
                             QString("30"))); // minutes

    // wav setup defaults
    m_tags["Wave"].append(MPair<QString, QString>(QString("samplesPerFrame"),
                                                  QString("8000")));
    m_tags["Wave"].append(MPair<QString, QString>(QString("bitsPerSec"),
                                                  QString("16")));
    m_tags["Wave"].append(MPair<QString, QString>(QString("fmtLength"),
                                                  QString("16")));
    m_tags["Wave"].append(MPair<QString, QString>(QString("audioFormat"),
                                                  QString("1")));
    m_tags["Wave"].append(MPair<QString, QString>(QString("channels"),
                                                  QString("1")));
    m_tags["Wave"].append(MPair<QString, QString>(QString("endiness"),
                                                  QString("LE")));

    // paths defaults - default directory support
    m_tags["Paths"].append(MPair<QString, QString>(QString("records"),
                                                  QString("records")));
    m_tags["Paths"].append(MPair<QString, QString>(QString("logs"),
                                                  QString("logs")));

    // records defaults
    m_tags["Record"].append(MPair<QString, QString>(QString("timestamp"),
                                                  QString("enabled")));
    m_tags["Record"].append(MPair<QString, QString>(QString("hasNumericConvention"),
                                                  QString("true")));

    // network defaults
    m_tags["Network"].append(MPair<QString, QString>(QString("transport"),
                                                  QString("udp")));
    m_tags["Network"].append(MPair<QString, QString>(QString("port"),
                                                  QString("1234")));

    // TODO: fix the speed check!!!!!
    // log defaults - speed is not checked yet!!!
    m_tags["Log"].append(MPair<QString, QString>(QString("name"),
                                                  QString("recorder.log")));
    m_tags["Log"].append(MPair<QString, QString>(QString("timestamp"),
                                                  QString("enabled")));
    m_tags["Log"].append(MPair<QString, QString>(QString("speed"),
                                                  QString("1000")));

    // heartbeat defaults - this is not used, just for some tests for now!
    m_tags["Heartbeat"].append(MPair<QString, QString>(QString("timeout"),
                                                  QString("2000")));
    m_tags["Heartbeat"].append(MPair<QString, QString>(QString("port"),
                                                  QString("5678")));
    m_tags["Heartbeat"].append(MPair<QString, QString>(QString("host"),
                                                  QString("127.0.0.1")));
    m_tags["Heartbeat"].append(MPair<QString, QString>(QString("enabled"),
                                                  QString("false")));

    // never fails always TRUE!
    return res;
}

/// no check for now
/// \brief RecorderConfig::getTagPairs
/// \param tag - tag name
/// \return  list of attributes
///
PairList &RecorderConfig::getTagPairs(const QString &tag)
{
    return m_tags[tag];
}

const MPair<QString, QString>
&RecorderConfig::getAttribPairFromTag(const QString &tag, const QString& attrib)
{
    PairList& plst = getTagPairs(tag);
    for(int i=0; i < plst.count(); ++i) {
        if (plst[i].m_type1 == attrib) {
            return plst.at(i);
        }
    }
    // return some error static object
    static MPair<QString, QString> none("", "");
    return none;
}

RecorderConfig &RecorderConfig::Instance()
{
    if (s_inst == nullptr) {
        s_inst = new RecorderConfig;
    }
    return *s_inst;
}

} // rec
} // plugin
