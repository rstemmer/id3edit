#!/usr/bin/env bash

HEADER="-I.."
LIBS="-lprinthex"

clang -DxDEBUG -Wno-multichar --std=gnu99 $HEADER $LIBS -O2 -g -o "encodingtest" ./encodingtest.c ../encoding.c
./encodingtest

# vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

