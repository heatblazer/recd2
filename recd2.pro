TEMPLATE = subdirs

DISTFILES += README.md \
             recorder-config.xml \# the config file
             recd2.pri \
                plugin-interface.h # plugin iface

SUBDIRS += \
    udp-server \
    udp-client \
    DFT \   #discrete fourier transform
    udp-streamer \ # udp treamer plugin
    NULL \
    recorder


