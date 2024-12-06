#!/bin/sh

INSTALLPATH=/usr/local/bin
mkdir -p "$INSTALLPATH"

for SOURCE in id3edit id3show id3frames id3dump ; do
    if [ "$(uname)" = "NetBSD" -o "$(uname)" = "OpenBSD" ]; then
        install -m 755 -o root $SOURCE $INSTALLPATH
    else
        install -m 755 -v -o root $SOURCE $INSTALLPATH
    fi
done

strip $INSTALLPATH/id3edit


# vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

