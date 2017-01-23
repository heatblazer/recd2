#ifndef IZ_FFT_H
#define IZ_FFT_H

namespace plugin {
    namespace fft {

    struct FFT
    {
        double pi;
        unsigned long fundamental_frequency;
        float* vector;
        FFT();
        ~FFT();
        void ComplexFFT(float data[], unsigned long number_of_samples, unsigned int srate, int sign);
    };

    } // fft
} // plugin


#endif // IZ_FFT_H
