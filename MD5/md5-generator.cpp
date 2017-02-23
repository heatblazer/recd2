/** A C like interface
 * for what a simple plugin can look like
 * it`s totally independant from the other
 * world. This plugin is not a real program
 * it`s a dummy to just test the plugin interfaces.
*/
#include "md5-generator.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#include <iostream>

// md5 crypto //
#include <openssl/md5.h>

// pass to plugins //
#include <QList>

#include "ipc-msg.h"
#include "recorder-config.h"

static const char* THIS_FILE = "md5-generator.cpp";

#define FRAME_SIZE 16


namespace plugin {
    namespace md5 {

        MD5Generator* MD5Generator::s_isntance = nullptr;

        void *MD5Generator::worker(void *pArgs)
        {
            MD5Generator* md5 = (MD5Generator*) pArgs;

            while (md5->isRunning) {

            }

            return (int*)0;
        }

        MD5Generator &MD5Generator::Instance()
        {
            if (s_isntance == nullptr) {
                s_isntance = new MD5Generator;
            }
            return *s_isntance;
        }

        void MD5Generator::init()
        {
            utils::IPC::Instance().sendMessage(THIS_FILE, "Init MD5 hasing plugin\n");
            MD5Generator* md5 = &Instance();
            md5->m_lock.init();
            md5->setThreadName("md5-worker");
            md5->createThread(128 * 1024, 20, MD5Generator::worker, md5);
        }

        int MD5Generator::put_ndata(void *data, int len)
        {
            (void) data; (void) len;
            return 0;
        }

        int MD5Generator::put_data(void *data)
        {
            MD5Generator* md = &Instance();
            QList<utils::sample_data_t>* ls =
                    (QList<utils::sample_data_t>*) data;

            if (md->s_iface.nextPlugin != nullptr) {
                md->s_iface.put_data(data);
            } else {
                ls->clear();
            }
            return 0;
        }

        void MD5Generator::set_name(const char* name)
        {
            MD5Generator* md5 = &MD5Generator::Instance();
            strncpy(md5->s_iface.name, name, 256);
        }

        const char* MD5Generator::get_name(void)
        {
            return Instance().s_iface.name;
        }

        void* MD5Generator::get_data()
        {
            return nullptr;
        }

        void MD5Generator::deinit()
        {
            utils::IPC::Instance().sendMessage(THIS_FILE, "Deinit Md5 hashing  plugin\n");
            Instance().isRunning = false;
            Instance().suspend(1000);
            Instance().join();
            Instance().closeThread();
        }

        int MD5Generator::main_proxy(int argc, char** argv)
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

        void MD5Generator::copy(const void* src, void* dest, int len)
        {
            (void)src;
            (void) dest;
            (void) len;
        }

        struct interface_t* MD5Generator::getSelf(void)
        {
            return &Instance().s_iface;
        }

        uint16_t MD5Generator::hwm(uint16_t vin)
        {
            uint16_t a = abs(vin);
            if (peekVal < a) {
                peekVal = a;
            }
        }

        MD5Generator::MD5Generator()
        {
            memset(&s_iface, 0, sizeof(s_iface));
        }

        MD5Generator::~MD5Generator()
        {

        }

    } // md5
} // plugin



const struct interface_t *get_interface()
{
    using namespace plugin::md5;

    struct interface_t* iface = MD5Generator::Instance().getSelf();
    iface->init = &MD5Generator::init;
    iface->deinit = &MD5Generator::deinit;
    iface->get_data = &MD5Generator::get_data;
    iface->put_data = &MD5Generator::put_data;
    iface->put_ndata = &MD5Generator::put_ndata;
    iface->main_proxy = &MD5Generator::main_proxy;
    iface->copy = &MD5Generator::copy;
    iface->setName = &MD5Generator::set_name;
    iface->getName = &MD5Generator::get_name;
    iface->getSelf = &MD5Generator::getSelf;
    iface->nextPlugin = nullptr;
    return iface;
}
