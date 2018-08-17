#include <errno.h> 
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>
#include <id3v2.h>
#include <id3v2frame.h>
#include <encoding/text.h>
#include <rawfile.h>
#include <printhex.h>
#include <stdbool.h>

#define VERSION "2.0.3"

int CopyArgument(char **dst, char *src);
int ProcessSetArgument(ID3V2 *id3v2, unsigned int ID, const char *argument, unsigned char encoding);
int ProcessGetArgument(const ID3V2 *id3v2, const unsigned int ID, const char *name);
int StoreArtwork(ID3V2 *id3v2, const char *storepath);
int ShowFramelist(const ID3V2 *id3v2);
int DumpFrame(const ID3V2 *id3v2, const char *frameid);
int GetEncoding(const char *codename, unsigned char *encoding);
void SafeFree(void* addr);
int ValidatePath(char **path, int accessmode); // makes path absolute and checks access accessmode: W_OK|R_OK


void PrintUsage()
{
    printf("\e[1;31mid3edit [\e[1;34m%s\e[1;31m]\e[0m\n", VERSION);
    printf("\n");
    printf("\e[1;37m id3edit \e[1;34m--help\n");
    printf("\e[1;37m id3edit \e[1;34m--version\n");
    printf("\e[1;37m id3edit \e[1;34moptions mp3file\n\n");

    // tag options
    printf("\e[1;34m Tag options:\n\e[1;37m");
    // setter
    printf("\t\e[1;46m  Option       \e[45m Arg. \e[44m  Description  \e[1;46m ID \e[45m  Example  \e[0m\n");
    printf("\t\e[1;36m --set-name    \e[35m name \e[34m Songname      \e[0;36mTIT2\e[35m Sonne    \n");
    printf("\t\e[1;36m --set-album   \e[35m name \e[34m Album name    \e[0;36mTALB\e[35m Mutter   \n");
    printf("\t\e[1;36m --set-artist  \e[35m name \e[34m Artist name   \e[0;36mTPE1\e[35m Rammstein\n");
    printf("\t\e[1;36m               \e[35m      \e[31m               \e[0;36mTPE2\e[35m          \n");
    printf("\t\e[1;36m --set-artwork \e[35m path \e[34m Artwork \e[1;31m¹     \e[0;36mAPIC\e[35m ./pic.jpg\n");
    printf("\t\e[1;36m --set-genre   \e[35m genre\e[34m Genre name    \e[0;36mTCON\e[35m Metal    \n");
    printf("\t\e[1;36m --set-release \e[35m year \e[34m Release year  \e[0;36mTYER\e[35m 2001 \e[1;30m(ID3v2.3.0)\n");
    printf("\t\e[1;36m               \e[35m      \e[34m               \e[0;36mTDRC\e[35m 2001 \e[1;30m(ID3v2.4.0)\n");
    printf("\t\e[1;36m --set-track   \e[35m track\e[34m Track number  \e[0;36mTRCK\e[35m 03/11    \n");
    printf("\t\e[1;36m --set-cd      \e[35m cd   \e[34m CD number     \e[0;36mTPOS\e[35m 1/1      \n");
    // getter
    printf("\t\e[1;36m --get-\e[35m$INFO         \e[34m Show \e[35m$INFO\e[34m={"
                                                                    "\e[0;35mname\e[1;34m,"
                                                                    "\e[0;35malbum\e[1;34m,"
                                                                    "\e[0;35martist\e[1;34m,"
                                                                    "\e[0;35mgenre\e[1;34m,"
                                                                    "\e[0;35mrelease\e[1;34m,"
                                                                    "\e[0;35mtrack\e[1;34m,"
                                                                    "\e[0;35mcd\e[1;34m}\n");
    printf("\t\e[1;36m --get-artwork \e[35m path \e[34m Store artwork      \n");
    printf("\t\e[1;36m --get-all     \e[35m      \e[34m Show all meta data \n");
    printf("\t\e[1;36m --get-frames  \e[35m      \e[34m Show all frames    \n");
    printf("\n");

    // Other options
    printf("\e[1;34m Other options:\n\e[1;37m");
    printf("\t\e[1;46m  Option       \e[45m Arg. \e[44m  Description      \e[1;45m  Example  \e[0m\n");
    printf("\t\e[1;36m --outfile     \e[35m path \e[34m Path to store mp3 \e[0;35m ./new.mp3\n");
    printf("\t\e[1;36m --dump        \e[35m ID   \e[34m Hex dump of a tag \e[0;35m TXXX     \n");
    printf("\t\e[1;36m --encoding    \e[35m code \e[34m Frame encoding \e[1;31m²  \e[0;35m utf-8    \n");
    printf("\n");

    // Flags
    printf("\e[1;34m Other options:\n\e[1;37m");
    printf("\t\e[1;46m  Flag         \e[1;44m  Description                       \e[0m\n");
    printf("\t\e[1;36m --readonly    \e[1;34m Do not Write to disk \n");
    printf("\t\e[1;36m --create      \e[1;34m Create ID3v2 Tag iff it\'s a bare mp3 file \n");
    printf("\t\e[1;36m --clear       \e[1;34m Remove all ID3v2 frames before adding new \n");
    printf("\t\e[1;36m --strip       \e[1;34m Remove whole ID3 Tag \e[1;30m(leaves a bare audio file) \n");
    printf("\t\e[1;36m --showheader  \e[1;34m Print details of the headers while reading \n");
    printf("\t\e[1;36m --force230    \e[1;34m Force ID3 v 2.3.0 when writing \e[1;31m³\n");
    printf("\t\e[1;36m --force240    \e[1;34m Force ID3 v 2.4.0 when writing \e[1;31m³\n");
    printf("\n");

    // Comments / footnotes
    printf("\e[1;31m  ¹ \e[1;34mSupport for \e[1;36mimage/jpeg\e[1;34m and \e[1;36mimage/png\e[1;34m. \e[1;30mFor more types create a bug report.\e[0m\n");
    printf("\n");

    printf("\e[1;31m  ² \e[1;34mChange the default text encoding from \e[1;36mUTF-16 with BOM\e[1;34m to one of the following:\e[0m\n");
    printf("\e[1;37m      \e[1;46m  Encoding  "
                         "\e[1;44m  Comment                           "
                         "\e[1;45m  Available since        \e[0m\n");
    printf("\e[1;31m      \e[1;36m UTF-16     "
                         "\e[1;34m UTF-16 with Byte Order Mark (BOM)  "
                         "\e[1;35m ID3v2.3.0 \e[1;30m(default) \e[0m\n");
    printf("\e[1;31m      \e[1;36m ISO8859-1  "
                         "\e[1;34m Known as latin-1 or falsely ASCII  "
                         "\e[1;35m ID3v2.3.0 \e[0m\n");
    printf("\e[1;31m      \e[1;36m UTF-16BE   "
                         "\e[1;34m UTF-16 Big Endian without BOM      "
                         "\e[1;35m ID3v2.4.0 \e[0m\n");
    printf("\e[1;31m      \e[1;36m UTF-8      "
                         "\e[1;34m Most common Unicode encoding       "
                         "\e[1;35m ID3v2.4.0 \e[1;30m(recommended)\e[0m\n");
    printf("\n");

    printf("\e[1;31m  ³ \e[1;33mIt is up to you to make sure all frames are conform to that version of the standard!\e[0m\n");
    printf("\e[1;33m    There are many differences in the details of ID3v2.3.0 and ID3v2.4.0!\e[0m\n");
    printf("\e[1;34m    You better only use it in the following combination: \e[1;36m--clear --create --force240\e[1;34m.\e[0m\n");
    printf("\e[1;30m    ID3v2.3.0 only allows UTF-16+BOM or ISO8859-1 encoded text.\e[0m\n");
    printf("\e[1;30m    Some frame IDs are different! (use --get-frames to check the new file)\e[0m\n");
    printf("\n");

    // Some warnings
    printf("\e[1;31m  * \e[1;33m\e[4mExperimental\e[0m\e[1;33m Software! - Do not do experiments with it!\e[0m\n");
    printf("\e[1;31m  * \e[1;33mArguments are \e[4mnot checked\e[0m\e[1;33m if they are valid (except pathes)!\e[0m\n");
    printf("\e[1;31m  * \e[1;37mDefault encoding for \"Text Information Frames\" is \e[4mUTF-16 with BOM\e[0m\e[1;37m.\e[0m\n");
}



int main(int argc, char *argv[])
{
    int exitcode = EXIT_FAILURE; // will be set to zero as last task - so all goto exit lead in returning EXIT_FAILURE

    // parse arguments
    if(argc <= 1)
    {
        PrintUsage();
        exit(EXIT_SUCCESS);
    }
    if(argc == 2) // maybe something special
    {
        if((strncmp("--help", argv[1], 20) == 0) || (strncmp("-h", argv[1], 20) == 0))
        {
            PrintUsage();
            exit(EXIT_SUCCESS);
        }
        else if((strncmp("--version", argv[1], 20) == 0) || (strncmp("-v", argv[1], 20) == 0))
        {
            printf("%s\n", VERSION);
            exit(EXIT_SUCCESS);
        }
        else
        {
            fprintf(stderr, "Invalid Argument - Input-path missing?\n");
            PrintUsage();
            exit(EXIT_FAILURE);
        }
    }

    char *newname   = NULL;
    char *newalbum  = NULL;
    char *newartist = NULL;
    char *newartwork= NULL;
    char *newgenre  = NULL;
    char *newrelease= NULL;
    char *newtracknr= NULL;
    char *newcdnr   = NULL;
    char *dumpframe = NULL;
    char *codename  = NULL; // Name of the encoding that shall be used for new frames
    char *mp3path   = NULL; // path to input file
    char *altpath   = NULL; // path to output file (if NULL, input = output)
    char *storeaw   = NULL; // path where the artwork from the mp3 file shall be stored
    bool readonly   = false;
    bool createtag  = false;
    bool cleartags  = false;
    bool striptag   = false;
    bool force230   = false;
    bool force240   = false;
    bool getframelist=false;
    bool getname    = false;
    bool getalbum   = false;
    bool getartist  = false;
    bool getgenre   = false;
    bool getrelease = false;
    bool gettracknr = false;
    bool getcdnr    = false;
    bool getall     = false;

    // start with argv[1], the first argument (argv[0] is the program file)
    // end one argument before the end because the last one is the mp3-file
    int argi;
    for(argi=1; argi<argc-1; argi++)
    {
        // check for options without argument
#define GETFLAG(v,n) if(strncmp((n), argv[argi], 20) == 0){(v) = true; continue;}
        GETFLAG(readonly,     "--readonly")
        GETFLAG(createtag,    "--create")
        GETFLAG(cleartags,    "--clear")
        GETFLAG(striptag,     "--strip")
        GETFLAG(force230,     "--force230")
        GETFLAG(force240,     "--force240")
        GETFLAG(OPT_PrintHeader, "--showheader")    // Global flag for the id3v2.c code
        GETFLAG(getframelist, "--get-frames")       // \_ Allow both options to show a list of frames
        GETFLAG(getframelist, "--get-framelist")    // /
        GETFLAG(getall,       "--get-all")
        GETFLAG(getname,      "--get-name")
        GETFLAG(getalbum,     "--get-album")
        GETFLAG(getartist,    "--get-artist")
        GETFLAG(getgenre,     "--get-genre")
        GETFLAG(getrelease,   "--get-release")
        GETFLAG(gettracknr,   "--get-track")
        GETFLAG(getcdnr,      "--get-cd")
 
        // check for options with argument
#define GETARG(v,n) if((v) == NULL && strncmp((n), argv[argi], 20) == 0) \
                    {CopyArgument(&(v), argv[++argi]); continue;}
        GETARG(newname,    "--set-name")
        GETARG(newalbum,   "--set-album")
        GETARG(newartist,  "--set-artist")
        GETARG(newartwork, "--set-artwork")
        GETARG(newgenre,   "--set-genre")
        GETARG(newrelease, "--set-release")
        GETARG(newtracknr, "--set-track")
        GETARG(newcdnr,    "--set-cd")
        GETARG(storeaw,    "--get-artwork")
        GETARG(codename,   "--encoding")
        GETARG(altpath,    "--outfile")
        GETARG(dumpframe,  "--dump")
        
        // invalid option
        fprintf(stderr, "Invalid Argument: \"%20s\"\n", argv[argi]);
        PrintUsage();
        goto exit;
    }

    if(argi > argc-1)
    {
        fprintf(stderr, "Missing input mp3file!\n");
        PrintUsage();
        goto exit;
    }

    // Now get the last argument - this shall be the path to the mp3 file
    CopyArgument(&mp3path, argv[argc-1]);

    // Convert relative paths to absolute paths and check access
    if(ValidatePath(&mp3path,    W_OK|R_OK) != 0) goto exit;
    if(ValidatePath(&newartwork,      R_OK) != 0) goto exit;
    //if(ValidatePath(&storeaw,            0) != 0) goto exit; FIXME: Does not work for non-existing files

    // OPEN
    int error;
    ID3V2 *id3v2 = NULL;
    error = ID3V2_Open(&id3v2, mp3path, createtag);
    if(error)
    {
        fprintf(stderr, "ID3V2_Open failed with error %i!\n", error);
        goto exit;
    }

    // Process program arguments
    unsigned char encoding = ID3V2TEXTENCODING_UTF16_BOM; // default ID3v2.3.0 encoding

    // Force ID3 version
    if(force230) id3v2->header.version_major = 3; // ID3v2.3.0
    if(force240) id3v2->header.version_major = 4; // ID3v2.4.0

    if(codename)     if(GetEncoding(codename, &encoding) != 0) goto exit;
    if(getframelist) if(ShowFramelist(id3v2)             != 0) goto exit;
    if(dumpframe)    if(DumpFrame(id3v2, dumpframe)      != 0) goto exit;

    // Get Tags
#define PROCESSGETARGUMENT(a,i,n)  if(getall || (a)) if(ProcessGetArgument(id3v2, (i), (n)) != 0) goto exit;
    if(getall
     ||getname
     ||getalbum
     ||getartist
     ||getrelease
     ||gettracknr
     ||getcdnr) printf("\e[1;46m  ID  \e[44m  Name    \e[46m  Value                 \e[0m\n");
    PROCESSGETARGUMENT(getname,    'TIT2', "\e[0;36m TIT2 \e[1;34m Name:     \e[36m")
    PROCESSGETARGUMENT(getalbum,   'TALB', "\e[0;36m TALB \e[1;34m Album:    \e[36m")
    PROCESSGETARGUMENT(getartist,  'TPE1', "\e[0;36m TPE1 \e[1;34m Artist:   \e[36m")
    PROCESSGETARGUMENT(getartist,  'TPE2', "\e[0;36m TPE2 \e[1;34m           \e[36m")
    PROCESSGETARGUMENT(getgenre,   'TCON', "\e[0;36m TCON \e[1;34m Genre:    \e[36m")
    PROCESSGETARGUMENT(getrelease, 'TYER', "\e[0;36m TYER \e[1;34m Release:  \e[36m")
    PROCESSGETARGUMENT(getrelease, 'TDRC', "\e[0;36m TDRC \e[1;34m           \e[36m")
    PROCESSGETARGUMENT(gettracknr, 'TRCK', "\e[0;36m TRCK \e[1;34m Track:    \e[36m")
    PROCESSGETARGUMENT(getcdnr,    'TPOS', "\e[0;36m TPOS \e[1;34m CD:       \e[36m")
    if(storeaw != NULL)
    {
        if(StoreArtwork(id3v2, storeaw) != 0) goto exit;
    }

    // Process some more flags
    if(cleartags) ID3V2_RemoveAllFrames(id3v2);

    // Set Tags
    if(ProcessSetArgument(id3v2, 'TIT2', newname,    encoding) != 0) goto exit;
    if(ProcessSetArgument(id3v2, 'TALB', newalbum,   encoding) != 0) goto exit;
    if(ProcessSetArgument(id3v2, 'TPE1', newartist,  encoding) != 0) goto exit;
    if(ProcessSetArgument(id3v2, 'TPE2', newartist,  encoding) != 0) goto exit;
    if(ProcessSetArgument(id3v2, 'TCON', newgenre,   encoding) != 0) goto exit;
    if(ProcessSetArgument(id3v2, 'TRCK', newtracknr, encoding) != 0) goto exit;
    if(ProcessSetArgument(id3v2, 'TPOS', newcdnr,    encoding) != 0) goto exit;
    if(ProcessSetArgument(id3v2, 'APIC', newartwork, encoding) != 0) goto exit;
    // Version depending tags
    if(id3v2->header.version_major == 3)
    {
        if(ProcessSetArgument(id3v2, 'TYER', newrelease, encoding) != 0) goto exit;
    }
    else if(id3v2->header.version_major == 4)
    {
        if(ProcessSetArgument(id3v2, 'TDRC', newrelease, encoding) != 0) goto exit;
    }

    // CLOSE
    if(readonly)
    {
        SafeFree(altpath);
        altpath = "/dev/null";  // if ID3V2_Close sees /dev/null, nothing will be stored
    }

    error = ID3V2_Close(id3v2, altpath, striptag);
    if(error)
    {
        fprintf(stderr, "ID3V2_Close failed with error %i!\n", error);
        goto exit;
    }

    // now the exit is wanted and not because of an occurred error
    exitcode = EXIT_SUCCESS;
exit:

    SafeFree(newname);
    SafeFree(newalbum);
    SafeFree(newartist);
    SafeFree(newartwork);
    SafeFree(newrelease);
    SafeFree(newgenre);
    SafeFree(newtracknr);
    SafeFree(newcdnr);
    SafeFree(mp3path);
    SafeFree(storeaw);
    SafeFree(codename);
    return exitcode;
}

//////////////////////////////////////////////////////////////////////////////

// Does not change anything when argument == NULL
int ProcessSetArgument(ID3V2 *id3v2, unsigned int ID, const char *argument, unsigned char encoding)
{
    if(argument == NULL)
        return 0;

    int error;
    switch(ID)
    {
        case 'TYER': // The 'Year' frame is a numeric string. It is always four characters long.
        case 'TDRC': // The 'Year' for ID3v2.4.0
        case 'TRCK': // E.g. "4/9"
        case 'TPOS': // E.g. "1/2"
        case 'TIT2':
        case 'TALB':
        case 'TPE1':
        case 'TPE2':
        case 'TCON':
            {
                error = ID3V2_SetTextFrame(id3v2, ID, argument, encoding);
                if(error)
                {
                    fprintf(stderr, "ID3V2_SetTextFrame for ID 0x%08X failed with error %i!\n", ID, error);
                    return -1;
                }
                break;
            }

        case 'APIC':
            {
                // Load Picture
                void  *picture = NULL;
                size_t picsize;
                error = RAWFILE_Read(argument, &picture, &picsize);
                if(error)
                {
                    fprintf(stderr, "RAWFILE_Read failed for \"%s\" with error %i!\n", argument, error);
                    return -1;
                }

                // Determine mime type
                const unsigned char *magicnumber;
                const char          *mimetype;
                magicnumber = (const unsigned char*)picture;
                if(magicnumber[0] == 0xFF
                && magicnumber[1] == 0xD8)
                    mimetype = "image/jpeg";
                else if(magicnumber[0] == 0x89 
                     && magicnumber[1] == 0x50
                     && magicnumber[2] == 0x4E
                     && magicnumber[3] == 0x47)
                    mimetype = "image/png";
                else
                {
                    SafeFree(picture);
                    fprintf(stderr, "Unknown image file type with (first 4 bytes: %02X, %02X, %02X, %02X! "
                            "Only png and jpeg files supported!\n", 
                            magicnumber[0],
                            magicnumber[1],
                            magicnumber[2],
                            magicnumber[3]);

                    return -1;
                }

                // Set APIC Frame
                error = ID3V2_SetPictureFrame(id3v2, 0x03 /*front cover*/, 
                                              mimetype, NULL, encoding,
                                              picture, picsize);

                // Free resources and check for error
                SafeFree(picture);
                if(error)
                {
                    fprintf(stderr, "ID3V2_SetPictureFrame failed with error %i!\n", error);
                    return -1;
                }

                break;
            }

        default:
            {
                    fprintf(stderr, "ID not supported as argument! (0x%08X)\n", ID);
                    return -1;
            }
    }

    return 0;
}

//----------------------------------------------------------------------------

int ProcessGetArgument(const ID3V2 *id3v2, unsigned int ID, const char *name)
{
    int    error;
    size_t bufferlimit = 1024;
    char  *textbuffer;
    textbuffer = malloc(bufferlimit);
    if(textbuffer == NULL)
    {
        fprintf(stderr, "Critical Error: malloc returned NULL!\n");
        return -1;
    }

    switch(ID)
    {
        case 'TYER': // The 'Year' frame is a numeric string. It is always four characters long.
        case 'TDRC': // The 'Year' for ID3v2.4.0
        case 'TRCK': // E.g. "4/9"
        case 'TPOS': // E.g. "1/2"
        case 'TIT2':
        case 'TALB':
        case 'TPE1':
        case 'TPE2':
        case 'TCON':
            {
                printf("%s", name);
                error = ID3V2_GetTextFrame(id3v2, ID, textbuffer, bufferlimit);
                if(error == ID3V2ERROR_FRAMENOTFOUND)
                {
                    printf("\e[0;33mFrame does not Exist\e[0m\n");
                }
                else if(error)
                {
                    fprintf(stderr, "ID3V2_SetTextFrame for ID 0x%08X failed with error %i!\n", ID, error);
                    SafeFree(textbuffer);
                    return -1;
                }
                else
                {
                    printf("%s\e[0m\n", textbuffer);
                }
                break;
            }

        default:
            {
                    fprintf(stderr, "ID not supported as argument! (0x%08X)\n", ID);
                    SafeFree(textbuffer);
                    return -1;
            }
    }

    SafeFree(textbuffer);
    return 0;
}

//----------------------------------------------------------------------------

int StoreArtwork(ID3V2 *id3v2, const char *storepath)
{
    char   *mimetype = NULL;
    void   *picture  = NULL;
    size_t  picsize  = 0;
    int     error;
    
    error = ID3V2_GetPictureFrame(id3v2, 0x03 /*Front Cover*/, &mimetype, NULL, &picture, &picsize);
    if(error)
        return -1;

    error = RAWFILE_Write(storepath, picture, picsize);
    if(error)
        return -1;

    SafeFree(mimetype);
    SafeFree(picture);
    return 0;
}

//----------------------------------------------------------------------------

int ShowFramelist(const ID3V2 *id3v2)
{
    // start printing frames
    ID3V2_FRAME  *frame;
    unsigned char majorversion;

    frame        = id3v2->framelist;
    majorversion = id3v2->header.version_major;

    // Headline
    printf("\e[1;37m ↱ "
           "\e[1;33m!\e[1;34m: Deprecated in ID3v2.4.0; "
           "\e[1;31m✘\e[1;34m: Undefined in ID3v2.3.0; "
           "\e[1;32m✔\e[1;34m: Valid\n");
    printf("\e[1;37m");
    printf("\e[44m   \e[46m  ID  \e[44m  Size  \e[44m  Flags  \e[44m Encoding                                \e[0m\n");

    while(frame)
    {

        // Print ID
        // Check if ID is deprecated in ID3v2.4.0
        if(majorversion == 4)
        {
            switch(frame->ID)
            {
                case 'EQUA':
                case 'IPLS':
                case 'RVAD':
                case 'TDAT':
                case 'TIME':
                case 'TORY':
                case 'TRDA':
                case 'TSIZ':
                case 'TYER':
                    printf("\e[1;33m ! "); // deprecated
                    break;
                default:
                    printf("\e[1;32m ✔ ");
                    break;
            }
        }
        // Check if ID is already defined for ID3v2.3.0
        else if(majorversion == 3)
        {
            switch(frame->ID)
            {
                case 'ASPI':
                case 'EQU2':
                case 'RVA2':
                case 'SEEK':
                case 'SIGN':
                case 'TDEN':
                case 'TDOR':
                case 'TDRC':
                case 'TDRL':
                case 'TDTG':
                case 'TIPL':
                case 'TMCL':
                case 'TMOO':
                case 'TPRO':
                case 'TSOA':
                case 'TSOP':
                case 'TSOT':
                case 'TSST':
                    printf("\e[1;31m ✘ "); // undefined for 2.3.0
                    break;
                default:
                    printf("\e[1;32m ✔ ");
                    break;
            }
        }

        // color shall indicate if supported or not
        switch(frame->ID)
        {
            case 'APIC':
            case 'TYER':
            case 'TDRC':
            case 'TRCK':
            case 'TPOS':
            case 'TIT2':
            case 'TALB':
            case 'TPE1':
            case 'TPE2':
            case 'TCON':
                printf("\e[1;36m"); // supported
                break;

            default:
                printf("\e[1;30m"); // not supported
                break;
        }

        printf(" %c%c%c%c ", ID3V2ID_TO_CHARS(frame->ID));

        // size
        printf("\e[1;34m %6i ", frame->size);

        // flags (flags are not supported now, if there are flags, print them red
        printf("%s 0x%04X ", (frame->flags == 0x0000)?"\e[1;34m":"\e[1;31m", frame->flags);

        // if frame is a text-frame, get some more infos - also for APIC
        unsigned char *data;
        data = (unsigned char*)frame->data;
        if((frame->ID >> 24) == 'T')
        {
            // encoding
            unsigned char encoding;
            encoding = data[0];
            if(encoding == ID3V2TEXTENCODING_ISO8859_1)
                printf("\e[1;30m ISO 8859-1 ");
            else if(encoding == ID3V2TEXTENCODING_UTF16_BOM)
                printf("\e[1;34m UTF-16 "); // 4 byte space for byteorder ("LE  ", "BE  ")
            else if(encoding == ID3V2TEXTENCODING_UTF16_BE)
                printf("\e[1;34m UTF-16BE ");
            else if(encoding == ID3V2TEXTENCODING_UTF8)
                printf("\e[1;34m UTF-8 ");
            else
                printf("\e[1;31m Invalid!   ");

            // BOM if UTF-16
            if(encoding == ID3V2TEXTENCODING_UTF16_BOM)
            {
                unsigned short *utf16data;
                unsigned short  byteorder;
                utf16data = (unsigned short*)&data[1];
                byteorder = utf16data[0];
                if(byteorder == UTF16BOM_BE)
                    printf("\e[0;36mBE  ");
                else if(byteorder == UTF16BOM_LE)
                    printf("\e[0;36mLE  ");
                else
                    printf("\e[1;31mBOM missing! ");

                // now check if there are more BOMs (caused by this fucking tool I used before)
                if(utf16data[1] == UTF16BOM_BE || utf16data[1] == UTF16BOM_LE)
                    printf("\e[1;33mMultiple BOM found! ");
            }

            // check if the used encoding is allows in the version of the standard claimed in the Tag Header
            if(majorversion < 4)
            {
                if(encoding == ID3V2TEXTENCODING_UTF16_BE || encoding == ID3V2TEXTENCODING_UTF8)
                    printf("\e[1;33mNot defined before ID3v2.4.0! \e[1;30m(Actual version: 2.%i.0)", majorversion);
            }

            // There should be no BOM within the UTF-16BE frames
            if(encoding == ID3V2TEXTENCODING_UTF16_BE)
            {
                unsigned short *utf16data;
                unsigned short firstword;
                utf16data = (unsigned short*)&data[1];
                firstword = utf16data[0];

                // now check if there is an unexpected BOM
                if(firstword == UTF16BOM_BE || firstword == UTF16BOM_LE)
                    printf("\e[1;33mUnexpected BOM found! ");
            }
        }
        else if(frame->ID == 'APIC')
        {
            char *mimetype;
            mimetype = (char*)(data + 1); // regarding to the specification this is a 0-terminated ISO 8859-1 string
            if(strncmp(mimetype, "image/jpg", 15) == 0)
                printf("\e[1;33m ");
            else if(strncmp(mimetype, "image/jpeg", 15) == 0)
                printf("\e[1;34m ");
            else if(strncmp(mimetype, "image/png", 15) == 0)
                printf("\e[1;34m ");
            else
                printf("\e[1;31m ");
            printf("%-15s ", mimetype);

            // Try to figure out what dimensions the picture has
            int error;
            void *picture;
            size_t picsize;
            error = ID3V2_GetPictureFrame(id3v2, 0x03, NULL, NULL, &picture, &picsize);
            if(error == ID3V2ERROR_NOERROR)
            {
                unsigned int width  = 0;
                unsigned int height = 0;
                if(strncmp(mimetype, "image/jpeg", 15) == 0
                 ||strncmp(mimetype, "image/jpg",  15) == 0)
                {
                    unsigned char *picptr = picture;
                    unsigned short chunksize;

                    // Check for expected SOI marker
                    if(picptr[0] != 0xFF || picptr[1] != 0xD8)
                        goto read_jpeg_failed;

                    // Jump over SOI marker
                    picptr  += 2;
                    picsize -= 2;

                    // Find SOF0 marker
                    while(picptr[0] != 0xFF || picptr[1] != 0xC0)
                    {
                        picptr   += 2; // \_ Jump over marker
                        picsize  -= 2; // /
                        chunksize = be16toh(*(unsigned short*)picptr);
                        if(chunksize >= picsize || chunksize == 0)
                            goto read_jpeg_failed;
                        picptr   += chunksize; // Jump over chunk
                        picsize  -= chunksize;
                    }

                    picptr   += 2; // Jump over marker
                    picptr   += 2; // Jump over size
                    picptr   += 1; // Jump over precision
                    width     = be16toh(*(unsigned short*)picptr);
                    picptr   += 2; // Jump over width
                    height    = be16toh(*(unsigned short*)picptr);
                }
                else if(strncmp(mimetype, "image/png", 15) == 0)
                {
                    char *picptr = picture;
                    picptr += 8; // 8 bytes signature
                    picptr += 4; // Chunk length (4 bytes, big-endian)
                    if(picptr[0] == 'I' && picptr[1] == 'H' && picptr[2] == 'D' && picptr[3] == 'R') // Header chunk
                    {
                        picptr += 4;
                        width   = be32toh(*(unsigned int*)picptr);
                        picptr += 4;
                        height  = be32toh(*(unsigned int*)picptr);
                    }
                }
read_jpeg_failed:
                free(picture);

                if(width == 0 || height == 0)
                    printf("\e[1;31m");
                else if(width > 10000 || height > 10000)
                    printf("\e[1;31m");
                else if(width != height)
                    printf("\e[1;33m");
                else
                    printf("\e[1;34m");
                printf("  %i × %i ", width, height);
            }

        }

        printf("\e[0m\n");

        // get next frame
        frame = (ID3V2_FRAME*)frame->next;
    }

    return 0;
}

//----------------------------------------------------------------------------

int DumpFrame(const ID3V2 *id3v2, const char *frameid)
{
    ID3V2_FRAME *frame;
    unsigned int ID = be32toh(*(unsigned int*)frameid);
    frame = id3v2->framelist;
    while(frame)
    {
        if(frame->ID == ID)
        {
            // Meta-data:
            printf("\e[1;34mID: \e[1;36m%c%c%c%c ", ID3V2ID_TO_CHARS(frame->ID));
            printf("\e[1;34m, Size: \e[0;36m%6i ", frame->size);
            printf("\e[1;34m, Flags: %s0x%04X\e[0m\n", (frame->flags==0x0000)?"\e[0;36m":"\e[0;31m", frame->flags);

            unsigned char *data = (unsigned char*)frame->data;
            data = (unsigned char*)frame->data;

            // Text Frame
            if((frame->ID >> 24) == 'T')
            {
                // encoding
                unsigned char encoding;
                encoding = data[0];

                printhex(frame->data, (frame->size > 0x100)?0x100:frame->size, 16, 
                        0, "\e[1;36m",          // encoding
                        1, (encoding==0x01)?"\e[1;35m":"\e[1;34m",  // BOM
                        3, "\e[1;34m", -1);
            }
            // Picture Frame
            else if(frame->ID == 'APIC')
            {
                // encoding
                unsigned char encoding = data[0];
                size_t        mimesize = strlen((char*)&data[1]);
                unsigned char pictype  = data[mimesize+2];
                unsigned int  descstart= mimesize+3;
                size_t        descsize = 0;

                // determine description length
                for(unsigned int i=descstart;; i++)
                {
                    if(encoding == ID3V2TEXTENCODING_UTF16_BOM || encoding == ID3V2TEXTENCODING_UTF16_BE) // 2 byte encodings
                    {
                        descsize += 2;
                        if(data[i] == 0x00 && data[i + 1] == 0x00)
                        {
                            break;
                        }
                        i++;
                    }
                    else // 1 byte encodings
                    {
                        descsize += 1;
                        if(data[i] == 0x00)
                        {
                            break;
                        }
                    }
                }

                printhex(data, 0x100, 16, 
                    0,                  "\e[1;36m", // encoding
                    1,                  "\e[1;35m", // mime-type
                    1+mimesize+1,       (pictype ==0x03)?"\e[1;32m":"\e[1;31m", // pictype
                    descstart,          "\e[0;35m", // description
                    descstart+descsize, "\e[1;34m", // pic-data
                    -1);
            }
            // Unknown Frame
            else
            {
                printhex(frame->data, (frame->size > 0x100)?0x100:frame->size, 16, 
                        0, "\e[1;34m", -1);
            }
            if(frame->size > 0x100)
                printf("\e[0;33mThere are more bytes…");
            printf("\n\e[0m");
        }

        frame = (ID3V2_FRAME*)frame->next;
    }

    return 0;
}

//////////////////////////////////////////////////////////////////////////////

// If encoding == NULL; only check if the name is valid
int GetEncoding(const char *codename, unsigned char *encoding)
{
    // Make string uniform
    size_t size;
    char  *name;
    size = strlen(codename) + 1;
    name = malloc(sizeof(char) * size);
    if(name == NULL)
    {
        fprintf(stderr, "Critical Error: malloc returned NULL!\n");
        return -1;
    }

    for(unsigned int i=0; i<size; i++)
    {
        name[i] = tolower(codename[i]);
        if(name[i] == '_') name[i] = '-';
        if(name[i] == ' ') name[i] = '-';
    }

    // Translate encoding name
    unsigned char code;

         if(strncmp(name, "iso8859-1",  size) == 0
          ||strncmp(name, "iso-8859-1", size) == 0
          ||strncmp(name, "latin-1",    size) == 0
          ||strncmp(name, "latin1",     size) == 0
          ||strncmp(name, "ascii",      size) == 0
          ) code = ID3V2TEXTENCODING_ISO8859_1;

    else if(strncmp(name, "utf16",    size) == 0
          ||strncmp(name, "utf-16",   size) == 0
          ||strncmp(name, "utf16bom", size) == 0
          ||strncmp(name, "utf16+bom", size) == 0
          ||strncmp(name, "utf-16+bom", size) == 0
          ) code = ID3V2TEXTENCODING_UTF16_BOM;

    else if(strncmp(name, "utf16be",   size) == 0
          ||strncmp(name, "utf-16be",  size) == 0
          ||strncmp(name, "utf16-be",  size) == 0
          ||strncmp(name, "utf-16-be", size) == 0
          ) code = ID3V2TEXTENCODING_UTF16_BE;

    else if(strncmp(name, "utf8",  size) == 0
          ||strncmp(name, "utf-8", size) == 0
          ) code = ID3V2TEXTENCODING_UTF8;

    else
    {
        fprintf(stderr, "Encoding (%s) not supported! Check help for valid encodings\n", codename);
        free(name);
        return -1;
    }

    // Return
    if(encoding != NULL)
        *encoding = code;

    free(name);
    return 0;
}

//////////////////////////////////////////////////////////////////////////////

int CopyArgument(char **dst, char *src)
{
    // Do nothing if we don't have a destination
    if(dst == NULL)
        return 0;

    // check if this argument is already in use
    if(*dst != NULL)
        return -1;

    size_t length = strlen(src);
    *dst = malloc(sizeof(char)*length+1); // +1 for the \0
    if(*dst == NULL)
    {
        fprintf(stderr, "Critical Error: malloc returned NULL!\n");
        return -1;
    }

    strncpy(*dst, src, length);
    return 0;
}

void SafeFree(void* addr)
{
    if(addr != NULL)
        free(addr);
}

int ValidatePath(char **path, int accessmode) // accessmode: W_OK|R_OK
{
    if(path == NULL)
        return 0;
    if(*path== NULL)
        return 0;

    // Convert relative paths to absolute paths
    char *retval = realpath(*path, NULL);
    if(retval == NULL)
    {
        fprintf(stderr, "Converting \"%s\" to a real path failed with error \"%s\"\n", *path, strerror(errno));
        return -1;
    }
    free(*path);
    *path = retval;

    // Check path
    if(access(*path, accessmode) == -1)
    {
        fprintf(stderr, "File %s does not exist or does not have R and/or W permissions!\n", *path);
        return -1;
    }

    return 0;
}

// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

