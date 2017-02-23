#ifndef MD5GENERATOR_H
#define MD5GENERATOR_H
#include "plugin-iface.h"
#include "utils.h"

#include <QList>

namespace plugin {

    namespace md5 {

    class MD5Generator : utils::PThread
    {
    public:
        static void* worker(void* pArgs);
        static MD5Generator& Instance();
        static void init();
        static void deinit(void);
        static void copy(const void* src, void* dst, int len);
        static int put_data(void* data);
        static int put_ndata(void* data, int len);
        static void* get_data(void);
        static void set_name(const char* name);
        static const char* get_name(void);
        static int main_proxy(int argc, char** argv);
        static struct interface_t* getSelf(void);


    private:

        MD5Generator();
        ~MD5Generator();
        static MD5Generator* s_isntance;
        bool isRunning;
        struct interface_t s_iface;

    };

    } // md5

} // plugin

#endif // MD5GENERATOR_H
