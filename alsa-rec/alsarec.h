#ifndef ALSAREC_H
#define ALSAREC_H

#include "thread.h"

namespace plugin {
namespace alsarec {

class AlsaRec
{
public:
    AlsaRec();
    ~AlsaRec();

    static void* worker(void* pArgs);

    void init();
    void createPlayback(const char* dev);
    void createCapture(const char* dev);
    void createPlayCapture(const char* pb, const char* cap);

private:
    Thread m_athread;
};


} // alsarec
} // plugin


#endif // ALSAREC_H
