#ifndef HELPER_H
#define HELPER_H

#include <QList>

#include <stdint.h>

#include "utils.h"


namespace plugin {

    namespace udp {
        class Server;

        class Helper : public QThread
        {
        public:
            static void* worker(void* pArgs);
            Helper();
            virtual ~Helper();
            void setPacketSize(int s);
            int16_t peek(int16_t* samples, int size);

        protected:
            virtual void run();
        private:
            int m_packSize;
            bool m_isRunning;
            int16_t m_peek;
            QList<frame_data_t> m_buffer;
            QMutex m_lock;
            friend class Server;
        };
    } // udp
} // plugin

#endif // HELPER_H
