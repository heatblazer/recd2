#ifndef CONSUMER_H
#define CONSUMER_H

#include "plugin-iface.h"
#include "utils.h"

namespace plugin {
#if 0

#endif

class Consumer : utils::PThread
{
public:
    Consumer();
    ~Consumer();

    static Consumer& Instance();
    static void* worker(void* pArgs);

    static void init();
    static void copy(const void*, void*, int len);
    static int put_ndata(void* data, int len);
    static int put_data(void* data);
    static void* get_data(void);
    static void  deinit();
    static int  main_proxy(int, char**);
    static void setName(const char*);
    static const char* getName(void);
    static struct interface_t* getSelf(void);

private:
    utils::frame_data_t test_data;
    struct interface_t s_iface;
    static Consumer* s_instance;
};

} // plugin

#endif // Consumer_H
