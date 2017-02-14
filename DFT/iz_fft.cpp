#include "iz_fft.h"

#include <QTimer>

#include <math.h>
#include <alloca.h>

#include "ipc-msg.h"
#include "utils.h"

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
    {
        pi = 4 * atan((double)1);

    }

    FFT::~FFT()
    {
    }

    void FFT::ComplexFFT(data_t data, int sign)
    {
        unsigned long n, mmax, m, j, istep, i;
        double wtemp, wr, wpr, wpi, wi, theta, tempr, tempi;

        float* vector = new float[2 * data.srate];
        if (vector == nullptr) {
            return;
        }
        memset(vector, 0, 2 * data.srate);
        //vector = new float[2 * srate];
        //put the real array in a complex array
        //the complex part is filled with 0's
        //the remaining vector with no data is filled with 0's
        for(n = 0; n < data.number_of_samples; n++) {
            if (n < data.number_of_samples) {
                vector[2 * n] = data.data[n];
            } else {
                vector[2 * n] = 0;
            }
            vector[2*n+1] = 0;
        }

        //binary inversion (note that the indexes
        //start from 0 witch means that the
        //real part of the complex is on the even-indexes
        //and the complex part is on the odd-indexes)
        n = data.srate << 1;
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
        for(i = 2; i <= data.srate; i += 2) {
            if ((pow(vector[i], 2) + pow(vector[i+1], 2)) >
                (pow(vector[fundamental_frequency], 2) + pow(vector[fundamental_frequency+1], 2))) {
                fundamental_frequency = i;
            }

        }

        fundamental_frequency = (long) floor((float) fundamental_frequency/2);
        // do something with the samples
        delete [] vector;
    }



    FFTPlugin* FFTPlugin::s_inst = nullptr;

    FFTPlugin &FFTPlugin::Instance()
    {
        if (s_inst == nullptr) {
            s_inst = new FFTPlugin;
        }
        return *s_inst;
    }

    // asynchro init
    void FFTPlugin::init()
    {
        FFTPlugin* r = &Instance();
        r->setObjectName("FFT thread");
        r->setStackSize(256 * 1024);
        r->m_isRunning = true;
        r->start();
    }

    void FFTPlugin::deinit()
    {
        utils::IPC::Instance().sendMessage("Deinitign FFT\n");
        FFTPlugin* r = &Instance();
        r->m_isRunning = false;
        r->wait(1000);
    }

    void FFTPlugin::copy(const void *src, void *dst, int len)
    {
        (void) src; (void) dst; (void) len;
    }

    int FFTPlugin::put_data(void *data)
    {
        if (data == nullptr) {
            return 1;
        }
        FFTPlugin* r = &Instance();
        QList<utils::sample_data_t>* ls = (QList<utils::sample_data_t>*)data;
        r->m_lock.lock();
        for(int i=0; i < ls->count(); ++i) {
            r->m_sampleBuffer.append(ls->at(i));
        }

        r->m_lock.unlock();

        if (r->iface.nextPlugin != nullptr) {
            // but before that do the fft
            r->iface.nextPlugin->put_data(data);
        } else {
            for(int i=0; i < ls->count(); ++i) {
                utils::sample_data_t s = ls->takeAt(i);
                if (s.samples != nullptr) {
                    delete [] s.samples;
                }
            }
        }
    }

    int FFTPlugin::put_ndata(void *data, int len)
    {
        FFTPlugin* r = &Instance();
        if (r->iface.nextPlugin != nullptr) {
            r->iface.nextPlugin->put_ndata(data, len);
        }
    }

    void *FFTPlugin::get_data()
    {
        return nullptr;
    }

    void FFTPlugin::setName(const char *name)
    {
        FFTPlugin* r = &Instance();
        strncpy(r->iface.name, name, sizeof(r->iface.name)/sizeof(r->iface.name[0]));
    }

    const char *FFTPlugin::getName()
    {
        FFTPlugin* r = &Instance();
        return r->iface.name;
    }

    int FFTPlugin::p_main(int argc, char **argv)
    {
        (void) argc; (void) argv;
        return 0;
    }

    interface_t *FFTPlugin::getSelf()
    {
        FFTPlugin* r = &Instance();
        return &r->iface;
    }

    void FFTPlugin::run()
    {
        FFTPlugin* r = &Instance();
        QList<utils::sample_data_t> dblBuff;

        do {
            usleep(100);
            r->m_lock.lock();
            while (!r->m_sampleBuffer.empty()) {
                dblBuff.append(r->m_sampleBuffer.takeFirst());
            }
            r->m_lock.unlock();

            while (!dblBuff.empty()) {
                utils::sample_data_t sdata = dblBuff.takeFirst();
                if (0/*bugg!!!*/) {
                    data_t d;
                    d.data = new float[sdata.size];
                    memcpy(d.data, sdata.samples, sdata.size);
                    d.number_of_samples = sdata.size;
                    d.srate = 16;
                    r->m_ftransform.ComplexFFT(d, 1);
                }
                // do something with analyze
                // r->m_ftransform.vector
            }

        } while (m_isRunning);
    }

    FFTPlugin::FFTPlugin(QThread *parent)
        : QThread(parent),
          m_isRunning(false)
    {

    }

    FFTPlugin::~FFTPlugin()
    {

    }

    } // fft
} // plugin


////////////////////////////////////////////////////////////////////////////////
const interface_t *get_interface()
{
    interface_t* piface = plugin::fft::FFTPlugin::Instance().getSelf();

    piface->init = &plugin::fft::FFTPlugin::init;
    piface->deinit = &plugin::fft::FFTPlugin::deinit;
    piface->copy = &plugin::fft::FFTPlugin::copy;
    piface->put_data = &plugin::fft::FFTPlugin::put_data;
    piface->put_ndata = &plugin::fft::FFTPlugin::put_ndata;
    piface->get_data = &plugin::fft::FFTPlugin::get_data;
    piface->main_proxy = &plugin::fft::FFTPlugin::p_main;
    piface->getSelf   = &plugin::fft::FFTPlugin::getSelf;
    piface->setName = &plugin::fft::FFTPlugin::setName;
    piface->getName = &plugin::fft::FFTPlugin::getName;
    piface->nextPlugin = nullptr;

    return plugin::fft::FFTPlugin::Instance().getSelf();
}
