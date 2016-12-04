#include "alsarec.h"

// ansi c //
#include <stdio.h>

namespace plugin {
namespace alsarec {

interface_t AlsaRec::s_iface;
AlsaRec* AlsaRec::s_inst = nullptr;

AlsaRec::AlsaRec()
{
}

AlsaRec::~AlsaRec()
{
}

AlsaRec &AlsaRec::Instance()
{
    if (s_inst == nullptr) {
        s_inst = new AlsaRec;
    }

    return *s_inst;
}

/// perform alsa setup here
/// \brief AlsaRec::init
///
void AlsaRec::init()
{

}

/// deinit alsa
/// \brief AlsaRec::deinit
///
void AlsaRec::deinit()
{

}

void AlsaRec::copy(const void *src, void *dest, int len)
{
    (void) src; (void) dest; (void) len;
}

int AlsaRec::put_ndata(void *data, int len)
{
    printf("AlsaRec: put data to ");
    if (s_iface.nextPlugin != nullptr) {
        puts("next plugin.");
        s_iface.nextPlugin->put_ndata(data, len);
    } else {
        puts("no one.");
    }
    return 0;

}

int AlsaRec::put_data(void *data)
{
    printf("AlsaRec: put data to ");
    if (s_iface.nextPlugin != nullptr) {
        puts("next plugin.");
        s_iface.nextPlugin->put_data(data);
    } else {
        puts("no one.");
    }
    return 0;
}

void *AlsaRec::get_data()
{
    return nullptr;
}

int AlsaRec::main_proxy(int argc, char **argv)
{
    (void) argc;
    (void) argv;
    printf("alsarec main: \n");
}

interface_t *AlsaRec::getSelf()
{
    return &s_iface;
}

} // alsarec
} // plugin

interface_t* get_interface()
{
    interface_t* pif = plugin::alsarec::AlsaRec::Instance().getSelf();

    pif->init = &plugin::alsarec::AlsaRec::init;
    pif->deinit = &plugin::alsarec::AlsaRec::deinit;
    pif->copy = &plugin::alsarec::AlsaRec::copy;
    pif->put_data = &plugin::alsarec::AlsaRec::put_data;
    pif->put_ndata = &plugin::alsarec::AlsaRec::put_ndata;
    pif->main_proxy = &plugin::alsarec::AlsaRec::main_proxy;
    pif->getSelf = &plugin::alsarec::AlsaRec::getSelf;

    pif->none = nullptr;
    pif->nextPlugin = nullptr;

    return plugin::alsarec::AlsaRec::Instance().getSelf();
}
