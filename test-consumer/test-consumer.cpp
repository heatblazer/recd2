/** A C like interface
 * for what a simple plugin can look like
 * it`s totally independant from the other
 * world. This plugin is not a real program
 * it`s a dummy to just test the plugin interfaces.
*/
#include "test-consumer.h"
#include "plugin-iface.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <iostream>
#include <QList>

#include "ipc-msg.h"
#include "recorder-config.h"
#include "utils.h"


static const char* THIS_FILE = "test-consumer.cpp";

namespace  plugin {

Consumer* Consumer::s_instance = nullptr;

void Consumer::init()
{
    utils::IPC::Instance().sendMessage(THIS_FILE, "Init Consumer...\n");
}

int Consumer::put_ndata(void *data, int len)
{
    if (Instance().s_iface.nextPlugin != NULL) {
        Instance().s_iface.nextPlugin->put_ndata(data, len);
    }
    return 0;
}

int Consumer::put_data(void *data)
{
    if (Instance().s_iface.nextPlugin != NULL) {
        Instance().s_iface.nextPlugin->put_data(data);
    } else {
        QList<sample_data_t>* ls = (QList<sample_data_t>*)data;
        ls->clear();
    }
    return 0;
}

void Consumer::setName(const char* name)
{
    strncpy(Instance().s_iface.name, name, 256);
}

const char* Consumer::getName(void)
{
    return Instance().s_iface.name;
}

void *Consumer::get_data()
{
    return &Instance();
}

void Consumer::deinit()
{
    utils::IPC::Instance().sendMessage(THIS_FILE, "Deinit Consumer \n");
}

int Consumer::main_proxy(int argc, char** argv)
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

void Consumer::copy(const void* src, void* dest, int len)
{
    (void)src;
    (void) dest;
    (void) len;
}

struct interface_t* Consumer::getSelf(void)
{
    return &Instance().s_iface;
}


Consumer::Consumer()
    : s_iface({0,0,0,
              0,0,0,
              0,0,0,
              0,{0},0})
{
}

Consumer::~Consumer()
{

}

Consumer &Consumer::Instance()
{
    if (s_instance == nullptr) {
        s_instance = new Consumer;
    }

    return *s_instance;
}

void *Consumer::worker(void *pArgs)
{
    Consumer* c = static_cast<Consumer*>(pArgs);
    (void)c;
    return nullptr;
}



} //plugin
const struct interface_t *get_interface()
{
    struct interface_t* s_iface = plugin::Consumer::Instance().getSelf();
    plugin::Consumer* p = &plugin::Consumer::Instance();
    s_iface->init = &p->init;
    s_iface->deinit = &p->deinit;
    s_iface->get_data = &p->get_data;
    s_iface->put_data = &p->put_data;
    s_iface->put_ndata = &p->put_ndata;
    s_iface->main_proxy = &p->main_proxy;
    s_iface->copy = &p->copy;
    s_iface->setName = &p->setName;
    s_iface->getName = &p->getName;
    s_iface->getSelf = &p->getSelf;
    s_iface->nextPlugin = nullptr;
    return s_iface;


}
