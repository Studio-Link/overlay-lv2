#!/bin/bash
gcc -g -Wall -shared -o studio-link.so -fPIC studio-link.c ../baresip/libbaresip.a \
    ../re/libre.a ../rem/librem.a ../opus/libopus.a -I../my_include -lssl -lcrypto

ldd studio-link.so
