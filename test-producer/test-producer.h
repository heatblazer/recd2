#ifndef PRODUCER_H
#define PRODUCER_H

#include "plugin-iface.h"
#include "utils.h"

namespace plugin {

#define SMPL_SIZE 100000
struct SMPL {
    char hdr[44];
    short int data[SMPL_SIZE];
};

class Producer
{
public:
    Producer();
    ~Producer();

    static Producer& Instance();
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
    frame_data_t test_data;
    struct interface_t s_iface;
    static Producer* s_instance;
    bool isRunning;
    SMPL samples;
};

} // plugin

#endif // PRODUCER_H
