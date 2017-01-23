#include "iz_fft.h"

#include <math.h>

#define SWAP(a, b) tempr = (a); (a) = (b); (b) = tempr

namespace plugin {
    namespace fft {


    /// perform complex fft over an array of samples
    /// \brief FFT::ComplexFFT
    /// \param data
    /// \param number_of_samples
    /// \param srate
    /// \param sign
    ///
    FFT::FFT()
        : vector(nullptr)
    {
        pi = 4 * atan((double)1);

    }

    FFT::~FFT()
    {
        if (vector != nullptr) {
            delete [] vector;
            vector = nullptr;
        }
    }

    void FFT::ComplexFFT(float data[], unsigned long number_of_samples, unsigned int srate, int sign)
    {
        unsigned long n, mmax, m, j, istep, i;
        double wtemp, wr, wpr, wpi, wi, theta, tempr, tempi;

        if (vector != nullptr) {
            delete [] vector;
        }

        vector = new float[2 * srate];
        //put the real array in a complex array
        //the complex part is filled with 0's
        //the remaining vector with no data is filled with 0's
        for(n = 0; n < number_of_samples; n++) {
            if (n < number_of_samples) {
                vector[2 * n] = data[n];
            } else {
                vector[2 * n] = 0;
            }
        }

        //binary inversion (note that the indexes
        //start from 0 witch means that the
        //real part of the complex is on the even-indexes
        //and the complex part is on the odd-indexes)
        n = srate << 1;
        j = 0;
        for(i=0; i < n/2; i+=2) {
            if (j > i) {
                SWAP(vector[j], vector[i]);
                SWAP(vector[j+1], vector[i+1]);

                if ((j/2) < (n/4)) {
                    SWAP(vector[(n-(i+2))], vector[(n-(j+2))]);
                    SWAP(vector[(n-(i+2))+1], vector[(n-(j+2))+1]);
                }
            }
            m =  n >> 1;
            while (m >=2 && j >= m) {
                j -= m;
                m >>= 1;
            }

            j+= m;
        }
        //end of the bit-reversed order algorithm

        // Danielson-Lanzcos algo:
        mmax = 2;
        while (n > mmax) {
            istep = mmax << 1;
            theta = sign * (2 * pi/mmax);
            wtemp = sin(0.5 * theta);
            wpr = -2.0 * wtemp * wtemp;
            wpi = sin(theta);
            wi = 0.0;
            for(m = 1; m < mmax; m += 2) {
                for(i = m; i <= n; i+= istep) {
                    j = i + mmax;
                    tempr = wr * vector[j-1] - wi * vector[j];
                    tempi = wr * vector[j] + wi * vector[j-1];
                    vector[j-1] = vector[i-1] - tempr;
                    vector[j] = vector[i] - tempi;
                    vector[i-1] += tempr;
                    vector[i] += tempi;
                }
                wr = (wtemp = wr) * wpr - wi * wpi + wr;
                wi = wi * wpr + wtemp * wpi + wi;
            }
            mmax = istep;
        }
        // end of algorithm

        fundamental_frequency = 0;
        for(i = 2; i <= srate; i += 2) {
            if ((pow(vector[i], 2) + pow(vector[i+1], 2)) >
                (pow(vector[fundamental_frequency], 2) + pow(vector[fundamental_frequency+1], 2))) {
                fundamental_frequency = i;
            }

        }

        fundamental_frequency = (long) floor((float) fundamental_frequency/2);
    }

    } // fft
} // plugin
