#include "types.h"
#if 0
#include <string.h>

utils::sample_data_t::sample_data_t()
    : samples(nullptr), size(0)
{
    samples = new short[320];
}

utils::sample_data_t::sample_data_t(uint32_t s)
    : samples(nullptr), size(s)
{
    if (s > 0) {
        samples = new short[size];
    }
}

utils::sample_data_t::sample_data_t(const utils::sample_data_t &ref)
{
    if (ref.samples != nullptr && ref.size > 0) {
        samples = new short[ref.size];
        size = ref.size;
        memcpy(samples, ref.samples, ref.size);
    }
}

utils::sample_data_t::~sample_data_t()
{
    if (samples != nullptr) {
        delete [] samples;
        samples = nullptr;
        size = 0;
    }
}
#endif
