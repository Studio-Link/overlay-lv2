#!/bin/bash -e

gcc -g -Wall -shared -o studio-link.so -fPIC studio-link.c ../baresip/libbaresip.a \
    ../re/libre.a ../rem/librem.a ../opus/libopus.a -I../my_include \
    ../openssl/libssl.a ../openssl/libcrypto.a -lm -lpthread -lz -ldl -lresolv

ldd studio-link.so
