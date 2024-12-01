<h3 align="center">id3edit</h3>

<div align="center">
  Status: 🟢  Active - License: GPL v3
</div>

---

<p align="center">id3edit is a command line ID3v2 tag editor and debugger
    <br/>
</p>

![Print all frames with details](screenshots/02.jpg?raw=true)

```bash
# Install clang and zlib
pacman -S clang zlib # Use the package manager of your distribution

# Install libprinthex
git clone https://github.com/rstemmer/libprinthex.git
cd libprinthex
./build.sh
sudo ./install.sh # Installs to /usr/local/{lib,include}

# Install id3edit
git clone https://github.com/rstemmer/id3edit.git
cd id3edit
./build.sh
sudo ./install.sh # Installs to /usr/local/bin

id3edit --readonly --showheader --get-all --get-frames bugtrigger.mp3
```

## Features

`id3edit` is a command line editor to edit and **debug** ID3v2 tags ([ID3v2.3.0](http://id3.org/id3v2.3.0) & [ID3v2.4.0](http://id3.org/id3v2.4.0-structure) of mp3 files with full Unicode support.

I separated id3edit from the [MusicDB Project](https://github.com/rstemmer/musicdb) to give it its own repository and own issue tracker.

**Its main features are:**

 * Show all frames of the ID3 tag (colloquial "mp3 tags")
 * Get/Add/Edit specific frames (see [Name Definitions](#name-definitions))
 * Get/Add artworks (Support for jpeg and png. Create an issue if you need more types.)
 * Remove frames or the whole tag
 * Print hex-dump of a specific frame
 * You can overwrite the input file or write to a new path
 * It is made to handle invalid tags and headers and debug them
 * Can print a detailed list of all frames with comments when they are invalid
 * Supports Unicode correctly!
 * It works from command line
 * Support for ID3v2.3.0 (most common) and ID3v2.4.0 (latest)
 * All encodings supported (ISO 8859-1, UTF-16 with BOM, UTF-16BE, UTF-8)
 * Partial support of extended header: CRC feature supported!

**Execution environment:**

 * Linux
 * MacOS
    * Ported by [Slava Baryshnikov (SeventyB)](https://github.com/SeventyB)
 * BSD
    * Ported by [Joel Demacky (tr0pe)](https://github.com/tr0pe)
 * Docker
    * Maintained by [Hendri Kurniawan (hckurniawan)](https://github.com/hckurniawan)
    * Dockerfile: [github.com/hckurniawan/docker-id3edit](https://github.com/hckurniawan/docker-id3edit)
    * Image: [hub.docker.com](https://hub.docker.com/r/hckurniawan/id3edit)


## Limitations

 * See Issue Tracker

## Name Definitions

 * **Tag:** The whole ID3 meta data of an mp3 file is called the Tag. A tag can have multiple _frames_.
 * **Frame:** A Frame is one piece of information of the audio file. For example the name of the song.
 * **[ID3] Tag Header:** A _Tag_ consists of a header followed by multiple _frames_.
 * **Frame Header:** Each _frame_ consists of a header and the information encoded in the frame. This information itself can have a further header. For example text frames start with a byte defining the encoding of the text.

## History of id3edit

The last three points form the feature list are the reason I developed this tool.
In the year 2013 I wanted to unify all tags of my music collection.
Therefore I need an ID3 editor I can call from a script to automate the tagging process.
Furthermore they had to be able to use Unicode encoded string (Think of Japanese bands).
I only found only a few editors that were able to work with Unicode at all.
Only one of them provided a command line interface.
This tool complete messed up all my tags because the promised Unicode support did not work.

That's why I needed a "ID3 Debugger" and this project was born.

Lessons learned: Also backup huge data collection and test foreign tools properly before using it in scripts :)

After separating id3edit from [MusicDB](https://github.com/rstemmer/musicdb) I reviewed the whole code and added some missing features.
Furthermore I added ID3v2.4.0 support.
So version 2.0.0 of id3edit was born.

## Examples

Here are some everyday examples as well as an example of how to fix a broken ID3 tag.

### Getting And Setting Frames

The following example first gets the name of audio file _example.mp3_, then changes its name to _"Example Song Name"_.

```bash
id3edit --get-name example.mp3
id3edit --set-name "Example Song Name" example.mp3
```

To write into a new file, you can use the `--outfile` option.
If all other meta data shall be removed use the `--clear` option.
The following example creates a clean set of meta data for a song file:

```bash
id3edit --set-name    "Example Song"         \\
        --set-album   "Example Album"        \\
        --set-artist  "Example Artist"       \\
        --set-artwork ~/pictures/example.jpg \\
        --set-genre   "Example Genre"        \\
        --set-release 2018                   \\
        --set-track   "13/42"                \\
        --set-cd      "1/1"                  \\
        --clean                              \\
        --encoding    utf-8                  \\
        --outfile Example\ Song.mp3          \\
        sourcefile.mp3

# Review changes
id3edit --get-all Example\ Song.mp3
```


### Debugging

Following scenario: The meta data of a song stored in an ID3 tag is invalid.
As an example I use a song from the band [Hämatom](https://en.wikipedia.org/wiki/H%C3%A4matom).
The file was originally tagged by a tool that claimed to support Unicode. As we discover in this section, this was not true :)

#### Printing the meta data

![Print all meta data](screenshots/01.jpg?raw=true)

 * `--get-all` prints all relevant frames.

Obviously there are some encoding problems and two different release dates.
We will look at the song name (`TIT2` frame) and genre (`TCON` frame) in detail later.

#### Getting details of the damaged frames

Now we can use `id3edit` to further inspect the file to figure out what's wrong:

![Print all frames with details](screenshots/02.jpg?raw=true)

 * `--showheader` prints lots of details of the ID3 header and the frame headers while reading the ID3 Tag.
 * `--get-frames` prints a list of all available frames including details of their size and encoding as well as other useful information.
    * The first column checks if the frames are defined or deprecated in the specific ID3v2 standard.
    * Gray IDs are IDs that are not directly supported by _id3edit_. (When they start with a `T` we can work with them anyway)
    * When there is a problem with the UTF-16 encoding, it gets mentioned in yellow.
    * If there is an artwork embedded, details like the mime type and dimension of the image are displayed.

The result of this first look into the details give us the following information:

 1. There is a frame not fully supported by _id3edit_ (`TSSE`). That's why they do not appear in the result of `--get-all`. (This is not a problem)
 2. The `TDRC` frame is not defined in ID3v2.3.0 version of the standard that is defined by the header.
 3. The claimed size of the ID3 Tag is greater than the actual size. The size given in the ID3 Tag Header will be adjusted. This must not be a sign of an invalid Tag. ID3 allows padding bytes that get striped away by _id3edit_.
 4. There are two BOMs (Byte Order Marks) in the `TCON` frame. This is an indicator that the mp3 file was tagged by a software that is not ID3v2 conform and/or has problems with Unicode encoded data. It also explains why the genre name in the first screenshot looks so strange.
 5. The artworks width is shorter than its height. That's just ugly but not really a problem with the ID3 Tag itself.

#### Check and repair song name


First lets check whats wrong with the song name (`TIT2`):

![Hex dump of song title](screenshots/03.jpg?raw=true)

 * `--dump $FRAMEID` print a hex dump of a specific frame

The previous analysis using `--get-framelist` told us, that the `TIT2` frame should have been _ISO 8859-1_ (also called _Latin-1_ or falsely _ASCII_) encoded.
This gets indicated by the first byte of the text frame (0x00 = ISO 8859-1). 
The actual text is _UTF-8_ encoded.
That is why the bytes 0xC3 and 0xA4 are interpreted as individual characters which is valid for the _ISO 8859-1_ encoding standard.
All other characters are correct because the have the same values in _UTF-8_ they have in _ISO 8859-1_.

The invalid frame can simply be fixed by calling `id3edit --set-name "Stay Kränk" invalid.mp3`.


#### Check the TCON problem

Just for demonstration lets see whats wrong with the `TCON` frame (Content Type - The genre of a song)

![Hex dump of a comment frame](screenshots/04.jpg?raw=true)

As already seen by the `--get-frames` output, there are two BOMs in the frame. The first byte 0x01 defines the encoding of the frame as _UTF-16_.
The [ID3v2 standard then expects a single byte order mark (BOM)](http://id3.org/id3v2.3.0#ID3v2_frame_overview).
The first one tells us, that a _UTF-16 LE_ (Little Endian) encoded string follows.
Then a new BOM appears defining an different flavor (_UTF-16 BE_ - Big Endian).
The text string itself is actually _UTF-16 BE_.
The second BOM violated the ID3v2 standard so that the bytes gets interpreted as an _UTF-16 LE_ encoded string.

Furthermore the 'ä' got replaced by a 'd' which indicated further Unicode problems in the deeper inside of the tool I used to tag the file initially.


## Installation

 1. Install dependencies:
    * `git`
    * `clang`
    * `zlib` (On Linux)
    * [libprinthex](https://github.com/rstemmer/libprinthex)
 2. You should check the `install.sh` script before executing. The default installation path is _/usr/local/bin_.
 3. Follow the following instructions:

```bash
# On Linux: Install clang and zlib
pacman -S git clang zlib # Use the package manager of your distribution
# On MacOS:
xcode-select --install # In case git/clang do not work yet

# Install libprinthex
git clone https://github.com/rstemmer/libprinthex.git
cd libprinthex
./build.sh
sudo ./install.sh # Installs to /usr/local/{lib,include}


# download
git clone https://github.com/rstemmer/id3edit.git
cd id3edit

# build
./build.sh

# install
sudo ./install.sh
```

## Usage

 * `id3edit --help` to see all available options.
 * `id3edit --version` to get the version number only, easy to parse without any escape sequences ;)

Shortcuts:
 
 * `id3show $FILE`: `id3edit --readonly --get-all $FILE`
 * `id3frames $FILE`: `id3edit --readonly --get-frames $FILE`
 * `id3dump $FRAMEID $FILE`: `id3edit --readonly --dump $FRAMEID $FILE`

## Contribute

In case you find a bug feel free to create an Issue.

If you have a mp3 file with invalid meta data and you cannot debug it with `id3edit`, create an Issue and append the invalid file to it (or at least the ID3 Tag).

Pull requests are welcome as well.

Bare mp3 files start with the magic number `0xFF, 0xFB`.
Sometimes the second byte is different, because this magic number is not really magic, it already contains information about the encoding.
In case you find a valid mp3 file and get the error `"ID: '???' (??????) not supported!` create an Issue and post the following output:

```bash
id3edit --readonly --showheader invalid.mp3
hexdump -C invalid.mp3 | head 
```

In general posing the following information can be helpful to find a bug:

```bash
id3edit --readonly --showheader --get-all --get-frames bugtrigger.mp3
hexdump -C bugtrigger.mp3 | head -n 50
```

