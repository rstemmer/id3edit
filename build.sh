#!/bin/sh

SOURCE=$(find . -type f ! -path './test/*' -name "*.c")
HEADER="-I. -I/usr/local/include"
if [ "$(uname)" = "Linux" -o "$(uname)" = "NetBSD" ]; then
    LIBS="-L/usr/local/lib -lprinthex -lz"
else
    LIBS="-L/usr/local/lib -lz -liconv -lprinthex"
fi
VERSION=$(head -n1 CHANGELOG | cut -d ' ' -f 1)

for c in $SOURCE ;
do    
    printf "\033[1;34mCompiling $c …\033[0m\n"
    clang -DxDEBUG -DVERSION="\"$VERSION\"" -Wno-multichar --std=gnu99 $HEADER -O2 -g -c -o "${c%.*}.o" $c
done


OBJECTS=$(find . -type f ! -path './test/*' -name "*.o")

clang -o id3edit $OBJECTS $LIBS

# vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

