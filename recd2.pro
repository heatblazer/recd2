TEMPLATE = subdirs

DISTFILES += README.md \
             recorder-config.xml \# the config file
             recd2.pri \
                plugin-interface.h # plugin iface

SUBDIRS += \
    udp-server \   # original program
    udp-client \   # test client
    DFT \          # discrete fourier transform
    udp-streamer \ # udp treamer plugin
    NULL \         # dummy test plugin
    recorder       # recorder plugin


