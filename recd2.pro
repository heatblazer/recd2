TEMPLATE = subdirs

DISTFILES += README.md \
             recorder-config.xml \  # the config file
             recd2.pri \
             plugin-interface.h     # plugin iface


SUBDIRS += \
    app \           # original program
    udp-client \   # test client
    DFT \          # discrete fourier transform
    udp-streamer \ # udp treamer plugin
    NULL \         # dummy test plugin
    recorder \       # recorder plugin
    alsa-rec \
    utils \
    test-producer \
    test-consumer

app.depends = utils
udp-streamer.depends = utils
recorder.depends = utils
alsa-rec.depends = utils
