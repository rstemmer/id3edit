# id3edit

`id3edit` is a tool to edit and __debug__ [ID3v2](http://id3.org/id3v2.3.0) tags of mp3 files.

I separated it from the [MusicDB Project](https://github.com/rstemmer/musicdb) to give it its own repository and own issue tracker.

__Its main features are:__

 * Show all frames of the ID3 tag (colloquial "mp3 tags")
 * Edit/Remove specific tags
 * Add/Remove artworks
 * Print hex-dump of a specific tag
 * You can overwrite the input file or write to a new path
 * It is made to handle invalid tags and headers and debug them
 * Supports Unicode correctly!
 * It works from command line

## Limitations

 * This tool only supports editing the most common tags. Nevertheless it can remove/debug unknown tags properly.
 * Furthermore it only writes UTF-16 (LE) encoded text. Reading other encoded (ISO-8859-1, UTF-16 (LE/BE), UTF-8) text is supported anyway.
 * It can read ID3v2.3.0 as well as ID3v2.4.0. The tags id3edit writes are ID3v2.3.0 compatible.
 * It only supports JPEG images for album covers.

## History of id3edit

The last three points form the feature list are the reason I developed this tool.
In the year 2013 I wanted to unify all tags of my music collection.
Therefore I need an ID3 editor I can call from a script to automate the tagging process.
I only found a few editors that were able to work with Unicode at all.
Only one of them provided a command line interface.
This tool complete messed up all my tags because the promised Unicode support did not work.
That's why I needed a "ID3 Debugger" and this project was born.
Lessons learned: Also backup huge data collection and test foreign tools properly before using it in scripts :)

## Installation

 - Install dependencies:
    * `clang`
    * [libprinthex](https://github.com/rstemmer/libprinthex)
 - You should check the `install.sh` script before executing. The default installation path is _/usr/local/bin_.
 - Follow the following instructions:

```bash
# download
git clone https://github.com/rstemmer/id3edit.git
cd id3edit

# build
./build.sh

# install
sudo ./install.sh
```

## Usage

 * ``id3edit --help`` to see all available options.
 * ``id3edit --version`` to get the version number only, and easy to parse ;)

