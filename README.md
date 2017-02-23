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
    15. Working on the ALSA lib. Now using QAudioRecorder.
    16. Now using QAudioInput: a class that can read directly from QIODevice and handle the
    data stream via a connection. The best approach so far. It needs a proper setup from the
    config file.
    17. I have to fix a bit the plugin interface API, some things are not to my liking, and
    a refractory is pending from this day: 24.01.2017!!!

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
    [Bug4] Don`t know if it`s bug or inproper ALSA config for the rec. Now I configure it outside,
    so I expect to be fixed soon.


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
    24.01.2017: A lot of bugfixes and memory leaks fixes.
    24.01.2017: A partially working (still not configured well) ALSA recorder.
    24.01.2017: A FFT plugin tested, still for now a memory crash is preset, but
    will fix it ASAP.
    24.01.2017: Yet more leaks and fixes are being made...
    24.01.2017: Some fixes: Now I don`t use QThread, rather I use pthreads.
    09.02.2017: Added test-producer and test-consumer plugins that are separate
    threads, for unit testing, perf testing and benchmarks.

## TESTING
    1. Use test-prducer to generate test data
    2. Use test-consumer to obtain the data
    3. You can perf-test the recorder if set the recorder plugin last and
    produce som data from test-producer
    4. udp-client project now streams file to the program for more testing.

## DTMF DETECTOR
    1. Dtmf detector plugin made by: Plyashkevich Viatcheslav <plyashkevich@yandex.ru>
    2. See the main.cpp in the DFT plugin in the project for example detection.

## MD5 CHECKSUM (Pending)
    1. Checksum generator.

## TOOLS
    1. topme - helper script to call "top -H -p" on a process name

<ilian.zapryanov@balkantel.net>

И.Z.
