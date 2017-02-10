/** A C like interface
 * for what a simple plugin can look like
 * it`s totally independant from the other
 * world. This plugin is not a real program
 * it`s a dummy to just test the plugin interfaces.
*/
#include "test-producer.h"
#include "plugin-iface.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <iostream>
#include <QList>

#include "ipc-msg.h"
#include "recorder-config.h"
#include "utils.h"

namespace  plugin {


Producer* Producer::s_instance = nullptr;

void Producer::init()
{
    Producer* p = &Instance();
    utils::IPC::Instance().sendMessage("Init Producer...\n");

    ((PThread*)p)->setName("producer-thread");
    p->create(128 * 1024, 10, Producer::worker, p);

}

int Producer::put_ndata(void *data, int len)
{
    if (Instance().s_iface.nextPlugin != NULL) {
        Instance().s_iface.nextPlugin->put_ndata(data, len);
    }
    return 0;
}

int Producer::put_data(void *data)
{
    if (Instance().s_iface.nextPlugin != NULL) {
        Instance().s_iface.nextPlugin->put_data(data);
    } else {
        QList<utils::sample_data_t>* ls = (QList<utils::sample_data_t>*)data;
        ls->clear();
    }
    return 0;
}

void Producer::setName(const char* name)
{
    strncpy(Instance().s_iface.name, name, 256);
}

const char* Producer::getName(void)
{
    return Instance().s_iface.name;
}

void *Producer::get_data()
{
    printf("NULL: Dummy get data!\n");
    return NULL;
}

void Producer::deinit()
{
    Instance().join();
    utils::IPC::Instance().sendMessage("Deinit Consumer\n");
}

int Producer::main_proxy(int argc, char** argv)
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

void Producer::copy(const void* src, void* dest, int len)
{
    (void)src;
    (void) dest;
    (void) len;
}

struct interface_t* Producer::getSelf(void)
{
    return &Instance().s_iface;
}


Producer::Producer()
    : s_iface({0,0,0,
              0,0,0,
              0,0,0,
              0,{0},0})
{
}

Producer::~Producer()
{

}

Producer &Producer::Instance()
{
    if (s_instance == nullptr) {
        s_instance = new Producer;
    }

    return *s_instance;
}

void *Producer::worker(void *pArgs)
{
    Producer* p = (Producer*) pArgs;
    utils::udp_data_t err_udp = {0, {0}, {{0}}};

    for(;;) {
        QList<utils::sample_data_t> ls;
        for(int i=0; i < 32; ++i) {
            utils::sample_data_t s = {0, 0};
            short smpl[16] = {0};
            s.samples = smpl;
            s.size = 16;
            for(int j=0; j < 16; ++j) {
                s.samples[j] = err_udp.data[i][j];
            }
            ls.append(s);
        }

        p->put_data((QList<utils::sample_data_t>*)&ls);
        p->suspend(10);
    }
}



} //plugin
const struct interface_t *get_interface()
{
    struct interface_t* s_iface = plugin::Producer::Instance().getSelf();
    plugin::Producer* p = &plugin::Producer::Instance();
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