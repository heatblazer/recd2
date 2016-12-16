Recorder
========

A wav recorder over udp streams.


## Build
   Added the dummy project for the gui interface of the recorder.

## Usage

    $ ./recd2 -d -c <path-to-conf-file>        # as daemon
    $ ./recd2 -c <path-to-conf-file>           # as normal app
    $ ./recd2 --config <path-to-conf-file>     # as normal app

## Config File
    Channels: count - number of channels up to 128
    HotSwap: maxSize in bytes or interval with timer.
    Wave: setup wav file - see the config file.
    Record: setup directories and other stuff for the recorder.
    Network: transport - can be udp or tcp, port to bind to.
    Log: name and path to log - unused for now.
    HeartBeat: send a keep alive packet to client if needed or to port.
    Plugin: name of the plugin, order - priority, enabled - disabled, path - fullpath.

    See the recorder-config.xml for example.
    The release will be preloaded with default settings if no options are given, or
    not proper setup is detected.
## Plugin API

    The program supports plugable programs, that can be loaded by describing them in the
    config file. When a programmer wants to support the program with a plugin, she must
    provide that interface:
        #ifdef __cplusplus
        extern "C"{
        #endif
        struct interface_t
        {
            void    (*init)(); // init the lib
            void    (*copy)(const void* src, void* dst, int len); // copy data
            int     (*put_ndata)(void* data, int len); // put N long data
            int     (*put_data)(void* data);           // put raw data
            void*   (*get_data)(void);                 // get worked data
            void    (*deinit)();                       // deinit lib
            int     (*main_proxy)(int, char**);        // pass caller args to lib
            void    (*setName)(const char*);           // sets the plugin name
            const char* (*getName)(void);              // gets the plugin name
            struct interface_t* getSelf();             // get this interface
            char name[256];                            // plugin name
            struct interface_t* nextPlugin;            // next loaded plugin
        };

        const struct interface_t* get_interface();
        #ifdef __cplusplus
        }
        #endif

    In brief, I expect the plugin to be able to init and deinit itself, and for now to
    be able to put and get some unknown data, and also to be able to support the C main
    function as the proxy I`ve preserved. So the main can be passed to the plugin. When
    I decide to change the interface, the contributors will be informed.

## TODO
    1. I have to remove the udp-client project when I am done with tests.
    2. I have to encrypt or add a check sum to the samples.
    3. I have to provide install options and "install howto" in build section.
    4. Crypto project and lib for hashing or crypting is still a stub.
    5. GUI for the recorder is stub yet.
    6. I have to add Build/Install instructions and options to the BUILD section.
    7. Daemon logging must support sys/log functionality. Not done yet.
    8. GUI must perform Goertzl algorithm for DTM and filtering the sample data.
    9. Meta info file.
    10. [DONE] Improve the log system`s messages.
    11. Add a binding/glue/shell like script support (qscript) to make things simple.
    12. [DONE] Remove plugin`s interface pritn messages.
    13. [DONE] Fix all warnings in all subprojects, please.
    14. [DONE] Most plugins depend on utils library. Organize project so everything is set and
    ready to use.
    15. Working on the ALSA lib.

## BUGS
    [Bug1] Strange bug as for 16.11.2016, when the program gives 100% cpu load on my other Fedora computer.
    [Fix Bug1] I've forgot a timer into server that has 10ms tick.
    [Bug2] Alsarec crashesh recorder when hotswap needs to happen. Possible problem is
    the unstoppable stream from the sounddevice, a buffering must be applied, probably.
    [Partial fix Bug2] The bug is from the size based hotswap. Timebased hotswap works fine.
    [Partial fix Bug2] I`ve fixed the nasty bug, and replaced fswatcher by a observer logic in QT api,
    by emmiting filechanged everytime QWav writes something. However the hotswap size based
    is still broken as f***!
    [Fix Bug2] Fixed the bug with the hotswap. Tested 24h size based hotswap, written over
    90Gb files over 50 000 files.
    [Bug3] Something happened to QWav class. I`ll fix it ASAP. Will revert it soon.


## NOTES
    31.10.2016: now correctly records the samples from the incomming device.
    31.10.2016: added libary project for crypting files or adding checksums.
    01.11.2016: added recorder-config.xml for more complex configure.
    01.11.2016: changed the logic and connection between `recorder` and `server`.
    02.11.2016: more WIP and concept fixes to the recorder and server.
    02.11.2016: improved configure of the application.
    02.11.2016: added QWav class based on QFile for writing WAV, unimplemented.
    02.11.2016: fixed a bug with the daemon recorder. But still more to be done.
    03.11.2016: added 24 hour test to the server, with custom client.
    03.11.2016: passed an 1 hour record test with packet sent at 1000 msec.
    08.11.2016: passed 24 and 72 h tests from my client program.
    08.11.2016: passed sine wave test from the device.
    08.11.2016: have a packet loss per random second or two.
    08.11.2016: added new concept for swapping files when size or time reach limit.
    08.11.2016: refractory and good coding practices added.
    08.11.2016: started implementation of QWav based on Qt and QFile
    08.11.2016: implemented and tested QWav. Will use it in the future.
    10.11.2016: fixed a small bug for the size of files read from XML file
    11.11.2016: added udp error packet with samples filled to max
    11.11.2016: started implementing the gui context for displaying wav samples.
    11.11.2016: animation frame of the pointerof the data.
    11.11.2016: simple concepts for gui`s context.
    11.11.2016: changed the name of the executable binary.
    11.11.2016: added test plugin architecture.
    11.11.2016: added UID to file: <channel-UID-timestamp.wav>
    11.11.2016: plugin manager test: must pass the full path to .so
    12.11.2016: tested loading few test plugins to the application. OK.
    12.11.2016: now loading 2 different plugins.. had some name mangling problems. Solved.
    13.11.2016: tested on windows host machine, besides unix daemon everything works fine.
    13.11.2016: project cleanup and file organizing.
    14.11.2016: now can configure paths for logging and recording
    14.11.2016: now records wavs to a directory specified.
    14.11.2016: log writer will be configured for speed of logging.
    15.11.2016: fixed the bug with the hotswap, now testFileWatcher() is pending deprecation.
    15.11.2016: old hotswap logic moved to the timer based hotswap.
    15.11.2016: config support time based hotswaps and the size based.
    16.11.2016: Now we have a Logger class accessible form everywhere.
    16.11.2016: I have to test in lab env. the QWavWriter.
    16.11.2016: Signals to be handled by sockstreams via IPC.
    16.11.2016: Added more messages to logs.
    16.11.2016: Tested time based hotswap and configuration from xml.
    17.11.2016: Testing time based hotswap in lab environment and also a simple filter as plugin.
    17.11.2016: Improved user menu and startup options aswell more friendly messages.
    18.11.2016: config improved: takes time in minutes and size in MB/GB.
    18.11.2016: pending test to sniffer port to test program packet loss counter.
    18.11.2016: Added WAV files with embeded META info for further research.
    21.11.2016: Added user server for further use.
    23.11.2016: Added GITHASH to the project to track versions.
    23.11.2016: Wav library has rename functionality and endian swaps.
    24.11.2016: Added a timer based transmission monitoring to the server.
    25.11.2016: Configurable channel count.
    27.11.2016: Tested flippin 32x16 sample data to 16x32... working ax expected.
    30.11.2016: Started to implement a firstclass plugin architecture.
    01.12.2016: Changed the plugin api to successfully chain multiple plugins.
    03.12.2016: Now program is plugin based. Recorder and server are separate.
    06.12.2016: Granulated project to smaller and added another to mailing list.
    09.12.2016: Now using IPC for messaging between plugins. Fast and simple.
    15.12.2016: Now plugins have get and set Name functions to find them later.

## KISS FFT
    KISS FFT - A mixed-radix Fast Fourier Transform based up on the principle,
    "Keep It Simple, Stupid."

        There are many great fft libraries already around.  Kiss FFT is not trying
    to be better than any of them.  It only attempts to be a reasonably efficient,
    moderately useful FFT that can use fixed or floating data types and can be
    incorporated into someone's C program in a few minutes with trivial licensing.

    USAGE:

        The basic usage for 1-d complex FFT is:

            #include "kiss_fft.h"

            kiss_fft_cfg cfg = kiss_fft_alloc( nfft ,is_inverse_fft ,0,0 );

            while ...

                ... // put kth sample in cx_in[k].r and cx_in[k].i

                kiss_fft( cfg , cx_in , cx_out );

                ... // transformed. DC is in cx_out[0].r and cx_out[0].i

            free(cfg);

        Note: frequency-domain data is stored from dc up to 2pi.
        so cx_out[0] is the dc bin of the FFT
        and cx_out[nfft/2] is the Nyquist bin (if exists)

        Declarations are in "kiss_fft.h", along with a brief description of the
    functions you'll need to use.

    Code definitions for 1d complex FFTs are in kiss_fft.c.

    You can do other cool stuff with the extras you'll find in tools/

        * multi-dimensional FFTs
        * real-optimized FFTs  (returns the positive half-spectrum: (nfft/2+1) complex frequency bins)
        * fast convolution FIR filtering (not available for fixed point)
        * spectrum image creation

    The core fft and most tools/ code can be compiled to use float, double,
     Q15 short or Q31 samples. The default is float.


    BACKGROUND:

        I started coding this because I couldn't find a fixed point FFT that didn't
    use assembly code.  I started with floating point numbers so I could get the
    theory straight before working on fixed point issues.  In the end, I had a
    little bit of code that could be recompiled easily to do ffts with short, float
    or double (other types should be easy too).

        Once I got my FFT working, I was curious about the speed compared to
    a well respected and highly optimized fft library.  I don't want to criticize
    this great library, so let's call it FFT_BRANDX.
    During this process, I learned:

        1. FFT_BRANDX has more than 100K lines of code. The core of kiss_fft is about 500 lines (cpx 1-d).
        2. It took me an embarrassingly long time to get FFT_BRANDX working.
        3. A simple program using FFT_BRANDX is 522KB. A similar program using kiss_fft is 18KB (without optimizing for size).
        4. FFT_BRANDX is roughly twice as fast as KISS FFT in default mode.

        It is wonderful that free, highly optimized libraries like FFT_BRANDX exist.
    But such libraries carry a huge burden of complexity necessary to extract every
    last bit of performance.

        Sometimes simpler is better, even if it's not better.

    FREQUENTLY ASKED QUESTIONS:
            Q: Can I use kissfft in a project with a ___ license?
            A: Yes.  See LICENSE below.

            Q: Why don't I get the output I expect?
            A: The two most common causes of this are
                    1) scaling : is there a constant multiplier between what you got and what you want?
                    2) mixed build environment -- all code must be compiled with same preprocessor
                    definitions for FIXED_POINT and kiss_fft_scalar

            Q: Will you write/debug my code for me?
            A: Probably not unless you pay me.  I am happy to answer pointed and topical questions, but
            I may refer you to a book, a forum, or some other resource.


    PERFORMANCE:
        (on Athlon XP 2100+, with gcc 2.96, float data type)

        Kiss performed 10000 1024-pt cpx ffts in .63 s of cpu time.
        For comparison, it took md5sum twice as long to process the same amount of data.

        Transforming 5 minutes of CD quality audio takes less than a second (nfft=1024).

    DO NOT:
        ... use Kiss if you need the Fastest Fourier Transform in the World
        ... ask me to add features that will bloat the code

    UNDER THE HOOD:

        Kiss FFT uses a time decimation, mixed-radix, out-of-place FFT. If you give it an input buffer
        and output buffer that are the same, a temporary buffer will be created to hold the data.

        No static data is used.  The core routines of kiss_fft are thread-safe (but not all of the tools directory).

        No scaling is done for the floating point version (for speed).
        Scaling is done both ways for the fixed-point version (for overflow prevention).

        Optimized butterflies are used for factors 2,3,4, and 5.

        The real (i.e. not complex) optimization code only works for even length ffts.  It does two half-length
        FFTs in parallel (packed into real&imag), and then combines them via twiddling.  The result is
        nfft/2+1 complex frequency bins from DC to Nyquist.  If you don't know what this means, search the web.

        The fast convolution filtering uses the overlap-scrap method, slightly
        modified to put the scrap at the tail.

    LICENSE:
        Revised BSD License, see COPYING for verbiage.
        Basically, "free to use&change, give credit where due, no guarantees"
        Note this license is compatible with GPL at one end of the spectrum and closed, commercial software at
        the other end.  See http://www.fsf.org/licensing/licenses

        A commercial license is available which removes the requirement for attribution.  Contact me for details.


    TODO:
        *) Add real optimization for odd length FFTs
        *) Document/revisit the input/output fft scaling
        *) Make doc describing the overlap (tail) scrap fast convolution filtering in kiss_fastfir.c
        *) Test all the ./tools/ code with fixed point (kiss_fastfir.c doesn't work, maybe others)

    AUTHOR:
        Mark Borgerding
        Mark@Borgerding.net


***

<ilian.zapryanov@balkantel.net>

И.Z.
