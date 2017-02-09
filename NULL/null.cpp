/** A C like interface
 * for what a simple plugin can look like
 * it`s totally independant from the other
 * world. This plugin is not a real program
 * it`s a dummy to just test the plugin interfaces.
*/
#include "plugin-iface.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <QList>

#include "ipc-msg.h"
#include "recorder-config.h"
#include "utils.h"

// explicitly null everything
static struct interface_t s_iface = {0,0,0,
                                     0,0,0,
                                     0,0,0,
                                     0,{0},0};


static void init()
{
    utils::IPC::Instance().sendMessage("Init null plugin\n");
}

static int put_ndata(void *data, int len)
{
    if (s_iface.nextPlugin != NULL) {
        s_iface.nextPlugin->put_ndata(data, len);
    }
    return 0;
}

static int put_data(void *data)
{
    if (s_iface.nextPlugin != NULL) {
        s_iface.nextPlugin->put_data(data);
    } else {
        QList<utils::sample_data_t>* ls = (QList<utils::sample_data_t>*)data;
        ls->clear();
    }
    return 0;
}

static void setName(const char* name)
{
    strncpy(s_iface.name, name, 256);
}

static const char* getName(void)
{
    return s_iface.name;
}

static void *get_data()
{
    printf("NULL: Dummy get data!\n");
    return NULL;
}

static void deinit()
{
    utils::IPC::Instance().sendMessage("Deinit null plugin\n");
}

static int main_proxy(int argc, char** argv)
{
    if (argc < 2) {
        return -1;
    } else {
        for(int i=0; i < argc; ++i) {
            if ((strcmp(argv[i], "-c") == 0) ||
                (strcmp(argv[i], "--config"))) {
                if (argv[i+1] != nullptr) {
                    utils::RecorderConfig::Instance().fastLoadFile(argv[i+1]);
                }
            }
        }
    }
    return 0;
}

static void copy(const void* src, void* dest, int len)
{
    (void)src;
    (void) dest;
    (void) len;
}

static struct interface_t* getSelf(void)
{
    return &s_iface;
}

const struct interface_t *get_interface()
{
    s_iface.init = &init;
    s_iface.deinit = &deinit;
    s_iface.get_data = &get_data;
    s_iface.put_data = &put_data;
    s_iface.put_ndata = &put_ndata;
    s_iface.main_proxy = &main_proxy;
    s_iface.copy = &copy;
    s_iface.setName = &setName;
    s_iface.getName = &getName;
    s_iface.getSelf = &getSelf;
    s_iface.nextPlugin = nullptr;
    return &s_iface;
}
