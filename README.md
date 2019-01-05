# recd2
============

32-64 channel recorder over UPD transport.

***

[5.01.2019]
## Update
Happy new year! Few minor updates:
1. `PThread` and `PMutex` are replaced by `QThread` and `QMutex` respectively.
2. Still have some minor issues to fix.

## Contributing

To start working over the project:

    # Clone
    $ cd /path to my project/
    $ git clone https://github.com/heatblazer/recd2

The project is over Qt5.8.0 LTS but is backward compatible and tested against
5.7.X and 5.6.X

### Warning!
QtScriptEngine is not supported in Qt5.11 and above. For simplicity, I've removed 
the ability to script the application. If you need a binding language,I will add
PySide support in the future.
Appy polly logies :)

### Development cycle

[PC]
The server is UDP based. It binds to a configured port and starts waiting for
incomming udp stream. There is an experimental TCP support. There is a helper
info server, which serveraddress and port can be configured. There is UDP message
server/logger and daemonizing tool, wich can be used with `-d` or `--daemon` and
a varying args after the name of the executable.

[HARDWARE]
The base data prodicer is 32 - 64 channels hardware with analogue to digital converter(ADC),
which sends a frame per clock to the network. The current frame is no more than described below:

```
   uint32_t counter
   uint8_t null[64]
   int16_t samples[512]
```

[PLUGIN]

The server supports flexible plugin API, which does not cares about how many pluggins
are loaded, for now it's concern is the order the plugins are chained. The current implemetation,
assumes that the first plugin loaded is called "the main producer" and the last one loaded is
"the final consumer", but the things can be very flexible and different. And also it can be used 
as simple logger, DSP converter, data streamer and whatever (as reported from few users).
Each plugin has it's own thread in which it works with a copy of the data (usually), but if it
has to mutate the data (assume adding some checksum or watermark) it can also modify it. 
The programmer is repsonsible to implement that interface:
```
    #ifdef __cplusplus
    extern "C"{
    #endif
    struct interface_t
    {
        void    (*init)();                          // init the lib
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
```

`init` - required function for all plugins, called from the plugin manager. Setups the plugin. Called automatically.

`deinit` - required function for all plugins, called from the plugin manager. Cleanup and finalizer. Called automatically.

`copy_data` - unused yet.

`put_data` - each plugin passes the data to the next with this call. Final plugin must not implement that, and can be used as
termination call. However, be sure to cleanup all data sent and do some finalizations.

`put_ndata` - identical to `put_data(...)` but with size. Currently most datas are wrapped with 
`QList<>` , which size is known via calls to`count()` or `size()`. It is considered `unused`,but the logic is 
the same as `put_data()`.

`get_data` - till recently unused, some users found it useful to return the data to the current plugin. 
I don't guarantee that is a correct way, but in some use cases it proved to be a good reason.
Ex:
```
    Recorder* r = pmanager.getInterfaceByName("recorder").get_data();
    ...
    void* Recorder::get_data()
    {
        return &Instance();
    }
```

`main_proxy` - proxy call to  `main` of the main application `app`. Used to pass args to the plugins if present. 
Usually if plugins have com line args or depend or specific `.conf` file. Called automatically.

`setName` - sets plugin name. Optional but good to use.

`getName` - takes plugin name. Optional but good to use.


[Utils]

`utils.pro` - static library used by all.


### Build

    `qmake-qt5`
    `make -j4`

### Deploy

Please refer to: [Qt Deploy](http://doc.qt.io/qt-5/linux-deployment.html) for static linking.

### Usage

Use the application as follows:

`$ ./recd2 -d -c <path-to-conf-file>` - starts and daemonizes with specific config file

`$ ./recd2 -c <path-to-conf-file>`  - runs as console app with config file

`$ ./recd2` - runs as console app with no config file. It's only for debuggin purposes and shall not
be used in release scenarios.


### Config File

Minimum config file per application is:
```
<Config />
```
This is NOT recommended since a failsafe config will be loaded. 

Optional configuration:

`<FrameData>` - tag desgribing the UDP frame data. Configurable. Very important, if missing a failsafe is loaded.
It's attributes are as follows:

`header` - 64 bytes. Message header. Here are messages to handle (channels with bytes and bit info).

`channels` - channels that shall be recorder.

`samples` - samples per channel. How 16 but samples will be in the channel, the max is 512 16bit integers (1024 bytes).
The configuration handles about offset and stride. 

The algo for extracting the data is: (changed)
```
    for i in CHANNELS:
        for j in SAMPLES_PER_CHAN:
            samples[j] = frame.data[j * CHANNELS + i];

```

`<HotSwap>` - tag describing the possibilty to stop the recording when a critical size reaches, or after time ellapses.
Importat since it's a main logic. There is a failsafe mode.

`timeBased=enabled` - if set to  `enabled`, the tag `maxSize` is omitted and `interval` is used to perform hotswap.

`timeBased=disabled` - if set to  `disabled`, the tag `maxSize` is used and `interval` is omitted. Describes size reached
when hotswap will be performed.

`maxSize` - look above. Agument is in megabytes `MB` (case insensitive).

`interval` - look above. The argument is in  `minutes`.

`<Wave>` - wav describing tag `wav` about the header. Important since if missing the recording will not work properly. 
`I know what I am doing` is required, if you don't know about `WAV` files, use the failsafe.
`samplesPerFrame` - `SPF` - freq of sampling `8Khz - 44khz`.

`bitsPerSec` - bits per second.

`fmtLenght` - format length, usually  `16`.

`audioFormat` - audio format. `1` is standart. 

`channels` - channels per file. In the case it's `1`, but can be resized to fit all in one file. Again 
`I know what I am doing` is required.

`endiness` - endiness. LE or BE. Not used for now.

`<Paths>` - App paths. Less important, if no tag, it will use some failsafe defaults (PWD),

`records="name"` - adds subdirectory name in which the records will be held.
	
`logs="name"` - adds subdirectoy `name` to the dir in which the logs will be held.


`<Network>` - Network type! Important. It has failsfe.

`transport` - transport layer. It's `UDP` but there is experimental `TCP`, which is not tested.

`port` - port for streams.

`<Log>` - Logging for the program. Important. It has failsafe.

`name` - name of file.

`timestamp` - add a timestapm.

`speed` - logging speed. This is a tweak option, since the logger is realtime, and it can afford to run a bit
slower than the main application.

`<LogServer>` - Message server. Optional. This is the mecahnism(consider `syslog` kind of) to log from all plugins to the program. It has failsafe.

`port` - server port.

`<HeartBeat>` - Keep alive or heartbeat. Unused for now.

`timepout` - heartbeat interval `hearbeat`.

`port` - heartbeat port

`host` - host for `heartbeat`. Ex.: `localdomain`.

`enabled` - enabled/disabled. Can be set in realtime. Disabled by default.


`<Plugin>` - Plugin concept. Not required. Unknown count. `WebConfig` adds the plugins in the real scenario.

`name` - name of the plugin. Desired to be unique. The name can be identical but there might be side effects.

`oreder` - reserved for future use.

`enabled` - unused. The idea is if enabled the manager will call init/deinit.

`path` - plugin path (`.so`). Absoulte file path.

`conf` - config file path. Example can be seen in the discrete fourier transform project : `DFT`.

Count of loaded plugins is not important. Here is an example of a simple loop in plugin:
Main thread of runner in producer:
```
    while(1) {
        // do something with local data
        put_data(local_data); // pass to other.
    }
```

Typical implementation of `put_data`:
```
    put_data(void* data) {
        LOCK();
        // copy to local_data - copy the data in local data, then unlock the thread
        UNLOCK();
        if (m_iface.next != nullptr) {
            m_iface.next->put_data(data); // виж има ли селдващ и подай нататък
        } else { // ако сме последни - почисти или финализирал.
            // clean data
            QList<int>* ls = (QList<int>*) data;
            ls->clear();
        }
   }
```


## BUGS
(fill in later)


## Builtin PLUGINS:

### TESTING

Test bundled plugins.
1. Use `test-prducer` to generate test data.
2. Use `test-consumer` to consume and work over the data.
3. `udp-client` - external test program that is similar to the  `HARDWARE DEV`, which throws `udp` 
packets and sends them for test purpose.
4. `NULL` - null plugin. It does nothing just tests the concept and mainly used for debugging.


### DTMF DETECTOR
Dulat tone modulation detector. Author: `Plyashkevich Viatcheslav <plyashkevich@yandex.ru>`.
Extended functionality and options for configuring. The perftests from the original author: `DFT/main.cpp`

### MD5 CHECKSUM (Pending)
[Pending]


## TOOLS
`topme` - helper python script to see the threads in the app, wraps: `top -H -p <proc name>` to monitor my app.
Ex.: `topme recd2`



<ilianzapryanov@abv.bg>

И.Z.
