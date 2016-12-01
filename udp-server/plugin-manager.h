#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

// qt stuff //
#include <QHash>
#include <QLibrary>
#include <QObject>

// local files //
#include "recorder-iface.h"

namespace iz {


// plugin manager shall load a library from
// .so or .dll file then resolves the deps
class RecPluginMngr
{
public:
    static bool loadLibrary(const QString& src, const QString& name);
    static const RecIface* getInterface(const QString& iface);
    static void unloadLibrary(const QString& lib);
    static const QList<RecIface>& listPlugins();

private:
    RecPluginMngr();
    ~RecPluginMngr();
    void resolve();
    static QHash<QString, RecIface> m_plugins;
    static QList<RecIface> m_listPlugins;
};

} // iz

#endif // PLUGINMANAGER_H
