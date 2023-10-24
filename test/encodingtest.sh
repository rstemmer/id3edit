#!/usr/bin/env bash

if [ "$(uname)" == "Darwin" ]; then
    HEADER="-I.. -I/usr/local/include"
    LIBS="-L/usr/local/lib -liconv -lprinthex"
else
    HEADER="-I.."
    LIBS="-lprinthex"
fi

clang -DxDEBUG -Wno-multichar --std=gnu99 $HEADER $LIBS -O2 -g -o "encodingtest" ./encodingtest.c ../encoding/text.c ../encoding/crc.c
./encodingtest

# vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

