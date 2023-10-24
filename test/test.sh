#!/usr/bin/env bash

SRCPNGAW="./tmp/source.png"
DSTPNGAW="./tmp/result.png"
SRCAW="./tmp/source.jpg"
DSTAW="./tmp/result.jpg"
SRC="./tmp/source.mp3"
DST="./tmp/result.mp3"

function CreateTestMP3 {

    base64 --decode > $SRC << EOF
//uUZAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAWGluZwAAAA8AAAACAAAE4ADj4+Pj
4+Pj4+Pj4+Pj4+Pj4+Pj4+Pj4+Pj4+Pj4+Pj4+Pj4+Pj4+Pj4+Pj4+Pj4+Pj////////////////
//////////////////////////////////////////////////8AAABQTEFNRTMuMTAwBLkAAAAA
AAAAADUgJAaQjQAB4AAABOClHWotAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA//vURAAAAioAXeAAAAA5
wAvMAAAAC4wteee9gQGnBW7495govjctbLXcgH1CAEATB8/E4IAg6DgIHMEMTg4CAIBjEAIO6wfD
HrD+D+CAJv/g4CAIAgGAfB8Hz//+CAIZQMfD//lwfB8H9HLb40kq8nBOD59YPg+XB8QAgciQEKwf
BDh7wfB8H/xACAIHMu/E4P//qBAMYIBj/Wa9Z/Ln8QA+H97e/M6SJuONIhFC3AZgMwmwuQ4lUdoc
RpE6A8IxOLX5rJjUFQVOwVBU6sFQVDTpUFQVBoGvBUFYKgqGuoGj0RB2oGga5UFQVBY9+IgZDXLH
vgqCuCoK/4iBk7/9W71Vv1JmlL0l1COiGmSkAhxpE6IMW0FSOEfs23NBSRLSoKgrBUFQ1BUFQ0oF
TpYGgZOqBoGmwVBUFga+oGgZBVwiBoFYieoGgaPSwdwWBoO/1gqCsqCuDLlA0DVQNB2WBoGgaBU7
/1gqCypMQU1FMy4xMDCqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqq
qqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqq
qqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqq
qqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqq
qqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqq
qqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqq
qqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqq
qqqqqqqqqqqqqqqq//sUZMsP8AAAaQAAAAgAAA0gAAABAAABpAAAACAAADSAAAAEqqqqqqqqqqqq
qqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqq
EOF

}
function CreateTestJPG {

    base64 --decode > $SRCAW << EOF
/9j/4AAQSkZJRgABAQEAYABgAAD/2wBDAAgGBgcGBQgHBwcJCQgKDBQNDAsLDBkSEw8UHRofHh0a
HBwgJC4nICIsIxwcKDcpLDAxNDQ0Hyc5PTgyPC4zNDL/2wBDAQkJCQwLDBgNDRgyIRwhMjIyMjIy
MjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjL/wAARCAABAAEDASIA
AhEBAxEB/8QAHwAAAQUBAQEBAQEAAAAAAAAAAAECAwQFBgcICQoL/8QAtRAAAgEDAwIEAwUFBAQA
AAF9AQIDAAQRBRIhMUEGE1FhByJxFDKBkaEII0KxwRVS0fAkM2JyggkKFhcYGRolJicoKSo0NTY3
ODk6Q0RFRkdISUpTVFVWV1hZWmNkZWZnaGlqc3R1dnd4eXqDhIWGh4iJipKTlJWWl5iZmqKjpKWm
p6ipqrKztLW2t7i5usLDxMXGx8jJytLT1NXW19jZ2uHi4+Tl5ufo6erx8vP09fb3+Pn6/8QAHwEA
AwEBAQEBAQEBAQAAAAAAAAECAwQFBgcICQoL/8QAtREAAgECBAQDBAcFBAQAAQJ3AAECAxEEBSEx
BhJBUQdhcRMiMoEIFEKRobHBCSMzUvAVYnLRChYkNOEl8RcYGRomJygpKjU2Nzg5OkNERUZHSElK
U1RVVldYWVpjZGVmZ2hpanN0dXZ3eHl6goOEhYaHiImKkpOUlZaXmJmaoqOkpaanqKmqsrO0tba3
uLm6wsPExcbHyMnK0tPU1dbX2Nna4uPk5ebn6Onq8vP09fb3+Pn6/9oADAMBAAIRAxEAPwD3+iii
gD//2Q==
EOF
}
function CreateTestPNG {

    base64 --decode > $SRCPNGAW << EOF
iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAACXBIWXMAAA7EAAAOxAGVKw4bAAAA
C0lEQVQImWP4DwQACfsD/eNV8pwAAAAASUVORK5CYII=
EOF
}

# MacOS comes with an old bash version that has issues with "\e":
# https://unix.stackexchange.com/questions/730403/echo-e-e-does-not-print-an-escape-from-bash-script-on-macos
# For this reason, all strings printed via echo will be treated via $"".
# Furthermore, Terminal.app does not support \e[s or u, instead \e7 or 8 is used for cursor position management.

function PrintHeader {
    echo $'\n\e[1;37mTest set: \e[1;36m'$@$'\e[0m'
}
function PrintTest {
    echo -n $'\e[1;37m[\e7 ]\e[1;34m '$@$'\e[0m'
}

function CheckResult {
    CHECKSUM=$(md5sum $DST 2> /dev/null | cut -d " " -f 1)

    if [ "$1" == "$CHECKSUM" ] ; then
        echo $'\e8\e[1;32m✔'
    else
        echo $'\e8\e[1;31m✘'
    fi
}
function CheckValues {
    if [ "$1" == "$2" ] ; then
        echo $'\e8\e[1;32m✔'
    else
        echo $'\e8\e[1;31m✘'
    fi
}


echo $'\e[1;37mRunning tests …\e[0m'
mkdir ./tmp
cp ../id3edit ./id3edit

./encodingtest.sh


PrintHeader "Creating ID3 Tags"
CreateTestMP3
PrintTest "Source file OK"
cp "$SRC" "$DST"
CheckResult "3064cf73aa8541e96201f8fab55293b2"

PrintTest "Create new ID3 Tag for new file"
CreateTestMP3
./id3edit --create --outfile $DST $SRC
CheckResult "2db1325baddc98e921587e07a724e0d8"

PrintTest "Create new ID3 Tag and overwrite source"
CreateTestMP3
./id3edit --create $SRC
cp "$SRC" "$DST"
CheckResult "2db1325baddc98e921587e07a724e0d8"

PrintTest "Create a clean ID3 Tag"
CreateTestMP3
./id3edit --create --clear $SRC
cp "$SRC" "$DST"
CheckResult "2db1325baddc98e921587e07a724e0d8"

PrintTest "Force ID3v2.4.0"
CreateTestMP3
./id3edit --create --force240 --outfile $DST $SRC
CheckResult "6f3816cf5708aea2f6f0e5cff1b3403e"

PrintTest "Force ID3v2.3.0"
./id3edit --create --force230 --outfile $DST $SRC
CheckResult "2db1325baddc98e921587e07a724e0d8"

PrintTest "Remove ID3v2 Tag"
./id3edit --strip $DST
CheckResult "3064cf73aa8541e96201f8fab55293b2"

PrintTest $'Try remove ID3v2 Tag from bare mp3 file \e[1;30m(Issue #20)'
CreateTestMP3
./id3edit --strip $SRC > /dev/null 2>&1
if [[ $? != 0 ]] ; then
    echo $'\e8\e[1;31m✘'
else
    cp "$SRC" "$DST"
    CheckResult "3064cf73aa8541e96201f8fab55293b2"
fi



PrintHeader "Creating and editing Text Frames"
CreateTestMP3

PrintTest "Create new ID3 Tag and set a Text Frame (with Unicode)"
./id3edit --create --set-name "This is a test 😈" --outfile $DST $SRC
CheckResult "8d4d5c79f926c6fdd7ed045738861035"

PrintTest "Replace Text Frame with shorter text"
./id3edit --set-name "Test" $DST
CheckResult "2754887be18efe5ff5e668806ce0b0d6"

PrintTest "Replace Text Frame with longer text"
./id3edit --set-name "A very very long text with lots of characters" $DST
CheckResult "a781e6ad1d326cb2a453063d678dfbfc"

PrintTest "Remove all Frames from ID3 Tag"
./id3edit --clear $DST
CheckResult "2db1325baddc98e921587e07a724e0d8"

PrintTest "Try replacing Text Frame with read-only flag"
./id3edit --set-name "Test" $DST
./id3edit --readonly --set-name "This is a test 😈" $DST
CheckResult "2754887be18efe5ff5e668806ce0b0d6"

PrintTest "Set UTF-16+BOM encoded Text Frame"
./id3edit --create --force230 --encoding UTF-16 --set-name "This is a test 😈" --outfile $DST $SRC
CheckResult "8d4d5c79f926c6fdd7ed045738861035"

PrintTest "Set UTF-16BE encoded Text Frame"
./id3edit --create --force240 --encoding UTF-16BE --set-name "This is a test 😈" $DST
CheckResult "ae0487260f5b150c468a0a9ea904f9bd"

PrintTest "Set UTF-8 encoded Text Frame"
./id3edit --create --force240 --encoding UTF-8 --set-name "This is a test 😈" $DST
CheckResult "99573302df02f4330546d30b20bff93a"

PrintTest "Set ISO 8859-1 encoded Text Frame"
./id3edit --create --force230 --encoding ISO8859-1 --set-name "This is ä test" $DST
CheckResult "362ef00025ee5d1475bd87fd4aab4e1f"

PrintTest "Set release year for ID3v2.3.0"
CreateTestMP3
./id3edit --create --force230 --set-release 2018 --outfile $DST $SRC
CheckResult "0855d28dbb241084557dbe6a23072ac8"

PrintTest "Set release year for ID3v2.4.0"
CreateTestMP3
./id3edit --create --force240 --set-release 2018 --outfile $DST $SRC
CheckResult "9df4c22b8b82db833ff7ce99b148c86c"

PrintTest $'Edit a frame without --create when no Tag exists \e[1;30m(Issue #25)'
CreateTestMP3
./id3edit --set-name "test" $SRC > /dev/null 2>&1 # An error message should be print
if [[ $? == 0 ]] ; then # And exit failure code should be returned
    echo $'\e8\e[1;31m✘'
else
    cp "$SRC" "$DST"
    CheckResult "3064cf73aa8541e96201f8fab55293b2"
fi


PrintHeader "Creating and editing artwork"
CreateTestMP3
CreateTestJPG
CreateTestPNG
./id3edit --create --clear $SRC

PrintTest "Add jpeg artwork for ID3v2.3.0"
./id3edit --set-artwork $SRCAW --outfile $DST $SRC
CheckResult "c2096703406b3e1b179dfe98a6ad3f6c"

PrintTest "Get jpeg artwork"
./id3edit --get-artwork $DSTAW $DST
SRCSUM=$(md5sum $SRCAW 2> /dev/null | cut -d " " -f 1)
DSTSUM=$(md5sum $DSTAW 2> /dev/null | cut -d " " -f 1)
CheckValues $SRCSUM $DSTSUM

PrintTest "Add jpeg artwork for ID3v2.4.0"
./id3edit --force240 --set-artwork $SRCAW --outfile $DST $SRC
CheckResult "8bed0327bfa6eb5eae80ed9d1e10b6c0"

PrintTest "Add png artwork for ID3v2.3.0"
./id3edit --set-artwork $SRCPNGAW --outfile $DST $SRC
CheckResult "832fcc2a6d9a55394fb27b0a72266c34"

PrintTest "Get png artwork"
./id3edit --get-artwork $DSTPNGAW $DST
SRCSUM=$(md5sum $SRCPNGAW 2> /dev/null | cut -d " " -f 1)
DSTSUM=$(md5sum $DSTPNGAW 2> /dev/null | cut -d " " -f 1)
CheckValues $SRCSUM $DSTSUM



PrintHeader "Creating and editing extended header"
CreateTestMP3

PrintTest "Add extended header with CRC for ID3v2.4.0"
./id3edit --create --clear --force240 --add-crc --encoding utf-8 --set-name "This is a test 😈" --outfile $DST $SRC
CheckResult "fa2ddd6d345694d0a238d00f98216fb7"

PrintTest "Remove extended header for ID3v2.4.0"
./id3edit --rm-exthdr $DST
CheckResult "99573302df02f4330546d30b20bff93a"

PrintTest "Add extended header with CRC for ID3v2.3.0"
./id3edit --create --clear --force230 --add-crc --set-name "This is a test 😈" --outfile $DST $SRC
CheckResult "c2d85c317db361f41766e059a323b791"

PrintTest "Remove extended header for ID3v2.3.0"
./id3edit --rm-exthdr $DST
CheckResult "8d4d5c79f926c6fdd7ed045738861035"


echo $'\n\e[1;37mRemoving tests …\e[0m'
rm $SRCAW
rm $DSTAW
rm $SRCPNGAW
rm $DSTPNGAW
rm $SRC
rm $DST
rm ./id3edit
rmdir ./tmp

# vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

