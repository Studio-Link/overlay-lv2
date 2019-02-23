#!/bin/bash -e

gcc -Wall -shared -o studio-link.so -fPIC studio-link.c ../baresip/libbaresip.a \
    ../re/libre.a ../rem/librem.a ../opus/libopus.a \
    ../openssl/libssl.a ../openssl/libcrypto.a -I../my_include \
     -lm -lpthread -lz -ldl -lresolv \
     -fvisibility=hidden -fdata-sections -ffunction-sections \
     -Wl,--gc-sections -Wl,-O1 -Wl,--as-needed -Wl,--strip-all

ldd studio-link.so
