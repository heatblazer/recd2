#include "plugin-iface.h"
#include <stdio.h>

static struct interface_t s_iface;

static void init()
{
    printf("Init NULL plugin\n");
}

static int put_ndata(void *data, int len)
{
    (void) data;
    (void) len;
    printf("NULL: Dummy put data!\n");
    return 0;
}


static int put_data(void *data)
{
    (void) data;
    printf("NULL: Dummy put data!\n");
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

    return &s_iface;
}

