/** Author:       Plyashkevich Viatcheslav <plyashkevich@yandex.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License
 * All rights reserved.
 */

#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include "DtmfDetector.hpp"
#include "DtmfGenerator.hpp"

#define MAX_DETECTORS 2
#define MAX_SAMPLES 102

const unsigned FRAME_SIZE = 160;

char dialButtons[16];

struct samples {
    samples(){}
    INT16 data[100000];
};

struct wav_file {
    char hdr[44];
    INT16 samples2[100000];
};

DtmfDetector dtmfDetector(FRAME_SIZE, 102);
DtmfGenerator dtmfGenerator(FRAME_SIZE, 40, 20);


int main()
{
    wav_file* w = new wav_file;
    samples* s = new samples;
    DtmfDetector** detectors = new DtmfDetector* [MAX_DETECTORS];
    for(int i=0; i < MAX_DETECTORS; ++i) {
        detectors[i] = new DtmfDetector(160, MAX_SAMPLES);
    }

    dialButtons[0] = '1';
    dialButtons[1] = '2';
    dialButtons[2] = '3';
    dialButtons[3] = 'A';
    dialButtons[4] = '4';
    dialButtons[5] = '5';
    dialButtons[6] = '6';
    dialButtons[7] = 'B';
    dialButtons[8] = '7';
    dialButtons[9] = '8';
    dialButtons[10] = '9';
    dialButtons[11] = 'C';
    dialButtons[12] = '*';
    dialButtons[13] = '0';
    dialButtons[14] = '#';
    dialButtons[15] = 'D';

    {
    FILE* fp = fopen("test.wav", "rb");
        if (!fp) {
            return 1;
        }
        size_t r = fread(w, 1, 100000, fp);
        fclose(fp);
    }





    while(true)
    {
#if 1
#if 0
        static int framenumber = 0;
        ++framenumber;
        dtmfGenerator.dtmfGeneratorReset();
        dtmfDetector.zerosIndexDialButton();
        dtmfGenerator.transmitNewDialButtonsArray(dialButtons, 16);
        while(!dtmfGenerator.getReadyFlag())
        {
            dtmfGenerator.dtmfGenerating(s->data); // Generating of a 16 bit's little-endian's pcm samples array
            dtmfDetector.dtmfDetecting(w->samples2); // Detecting from 16 bit's little-endian's pcm samples array
        }
#endif
        dtmfDetector.dtmfDetecting(w->samples2); // Detecting from 16 bit's little-endian's pcm samples array

        if(dtmfDetector.getIndexDialButtons() < 1)
        {
            printf("Error of a number of detecting buttons \n");
            continue;
        }

        for(int ii = 0; ii < dtmfDetector.getIndexDialButtons(); ++ii)
        {
            if(dtmfDetector.getDialButtonsArray()[ii] != dialButtons[ii])
            {
                printf("Error of a detecting button \n");
                continue;
            } else {
                printf("We got: [%c]\n", dialButtons[ii]);
            }
        }
    }
#endif
#if 0

        for(int i=0; i < MAX_DETECTORS; ++i) {
            INT16* s = (INT16*) w->samples2;
            detectors[i]->dtmfDetecting(s);
            if(detectors[i]->getIndexDialButtons() != 16)
            {
                printf("Error of a number of detecting buttons \n");
                continue;
            }

            for(int ii = 0; ii < detectors[i]->getIndexDialButtons(); ++ii)
            {
                if(detectors[i]->getDialButtonsArray()[ii] != dialButtons[ii])
                {
                    printf("Error of a detecting button \n");
                    continue;
                } else {
                    printf("We got: [%c]\n", dialButtons[ii]);
                }
            }
        }

//        printf("Success in frame: %d \n", framenumber);
    }
#endif

    delete w;
    for(int i=0; i < MAX_DETECTORS; ++i) {
        delete detectors[i];
    }
    delete s;
    delete [] detectors;
    return 0;
}


