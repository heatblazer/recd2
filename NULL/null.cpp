#include "plugin-iface.h"
#include <stdio.h>
#include <stdint.h>

struct udp_data_t
{
    uint32_t    counter;
    uint8_t     null_bytes[32];
    int16_t    data[32][16];
};


static struct interface_t s_iface;

static void init()
{
    printf("Init NULL plugin\n");
}

static int put_ndata(void *data, int len)
{
    printf("NULL: put data to ");
    udp_data_t* udp = (udp_data_t*) data;
    if (s_iface.nextPlugin != NULL) {
        puts("next plugin.");
        s_iface.nextPlugin->put_ndata((udp_data_t*)udp, len);
    } else {
        puts(" no one.");
    }
    // remove it.. why returning...
    return 0;
}


static int put_data(void *data)
{
    udp_data_t* udp = (udp_data_t*) data;
    printf("NULL: put data to ");
    if (s_iface.nextPlugin != NULL) {
        puts("next plugin.");
        s_iface.nextPlugin->put_data((udp_data_t*)udp);
    } else {
        puts("no one.");
    }
}

static void *get_data()
{
    printf("NULL: Dummy get data!\n");
    return NULL;
}

static void deinit()
{
    printf("NULL: dummy deinit!\n");
}

static int main_proxy(int argc, char** argv)
{
    (void) argc;
    (void) argv;
    printf("NULL: No args main...\n");
    return 0;
}

static void copy(const void* src, void* dest, int len)
{
    (void)src;
    (void) dest;
    (void) len;
    printf("NULL: Dummy copy...\n");
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
    s_iface.getSelf = &getSelf;
    s_iface.none = nullptr;
    s_iface.nextPlugin = nullptr;

    return &s_iface;
}

