#include "plugin-manager.h"
#include "defs.h"

#include <stdio.h>
#include "utils.h"

static const char* THIS_FILE = "plugin-manager.cpp";

namespace recd {

typedef struct interface_t* (*get_interface)();
InterfaceList RecPluginMngr::m_pluginLinks;

/// Plugin loader
/// \brief RecPluginMngr::loadLibrary
/// \param src - must be a config file attribs
/// \return OK if loaded, fale else
///
bool RecPluginMngr::loadLibrary(const QString &src, const QString& name)
{
    bool res = false;
    QLibrary plugin(src);
    res = plugin.load();
    if (!res) {
        utils::Logger::Instance().logMessage(THIS_FILE, plugin.errorString().toStdString().data());
        return res;
    }

    bool load_all_res = false;
    interface_t iface;
    get_interface lib_ifaceCb = (get_interface) plugin.resolve("get_interface");

    if (lib_ifaceCb != nullptr) {
        struct interface_t* lib_symbols = lib_ifaceCb();
        if (lib_symbols->deinit == nullptr
            || lib_symbols->init == nullptr
            || lib_symbols->get_data == nullptr
            || lib_symbols->put_data == nullptr
            || lib_symbols->put_ndata == nullptr
            || lib_symbols->main_proxy == nullptr
            || lib_symbols->copy == nullptr
            || lib_symbols->getSelf == nullptr)
        {
            load_all_res = false;

        } else {
            iface.init = lib_symbols->init;
            iface.deinit = lib_symbols->deinit;
            iface.get_data = lib_symbols->get_data;
            iface.put_data = lib_symbols->put_data;
            iface.put_ndata = lib_symbols->put_ndata;
            iface.main_proxy = lib_symbols->main_proxy;
            iface.getSelf = lib_symbols->getSelf;
            load_all_res = true;

            // not working yet
            snprintf(iface.name, sizeof(iface.name), "%s",
                     name.toStdString().data());
            // link the plugins from here
            m_pluginLinks.put(iface.getSelf());
        }

    } else {
        static char err [256] = {0};
        snprintf(err, sizeof(err), "\n%s\n", plugin.errorString().toStdString().data());
        utils::Logger::Instance().logMessage(THIS_FILE, err);
    }

    return load_all_res;
}

/// get a pointer to the iterface from the
/// map, by a given name:
/// ex. getInterface("Test!")->init();
/// \brief RecPluginMngr::getInterface
/// \param name of loaded plugin
/// \return
///
const interface_t *RecPluginMngr::getInterface(const QString &iface)
{
#if 0
    if( m_plugins.find(iface) != m_plugins.end() ) {
        return &m_plugins[iface];
    } else {
        return nullptr;
    }
#endif
    interface_t* it = m_pluginLinks.getFront();
    while (it != nullptr) {
       if (QString(it->name) == iface) {
           return it;
       }
       it = it->nextPlugin;
    }
    return nullptr;
}

/// Unused for now
/// \brief RecPluginMngr::unloadLibrary
/// \param lib
///
void RecPluginMngr::unloadLibrary(const QString &lib)
{
    (void) lib;
}

/// new concept to deprecate the above
/// \brief RecPluginMngr::getPluginList
/// \return
///
InterfaceList &RecPluginMngr::getPluginList()
{
    return m_pluginLinks;
}

/// nothing by default
/// \brief RecPluginMngr::RecPluginMngr
///
RecPluginMngr::RecPluginMngr()
{
}

///
/// \brief RecPluginMngr::~RecPluginMngr
///
RecPluginMngr::~RecPluginMngr()
{
}

} // recd
