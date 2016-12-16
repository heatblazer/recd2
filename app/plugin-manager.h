#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

// qt stuff //
#include <QHash>
#include <QLibrary>
#include <QObject>

#include "defs.h"

namespace recd {

// plugin manager shall load a library from
// .so or .dll file then resolves the deps
class RecPluginMngr
{
public:
    static bool loadLibrary(const QString& src, const QString& name);
    static const interface_t *getInterface(const QString& iface);
    static void unloadLibrary(const QString& lib);
    static InterfaceList &getPluginList();

private:
    RecPluginMngr();
    ~RecPluginMngr();
    void resolve();
    static InterfaceList m_pluginLinks;
};

} // recd

#endif // PLUGINMANAGER_H
