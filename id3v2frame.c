#include <errno.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <id3v2.h>
#include <id3v2frame.h>
#include <encoding/text.h>

#ifdef DEBUG
#include <printhex.h>
#endif

int ID3V2_GetTextFrame(const ID3V2 *id3v2, unsigned int ID, char *utf8text, size_t bufferlimit)
{
    int error;

    // Get raw data
    size_t         framesize;
    unsigned char *framedata;
    error = ID3V2_GetFrame(id3v2, ID, &framesize, (void**)&framedata);
    if(error)
        return error;

    if(utf8text == NULL)
    {
        free(framedata);
        return ID3V2ERROR_NOERROR;
    }

    // First byte defines the encoding
    unsigned char encoding;
    encoding = *framedata;

    // Rest is the encoded text that shall be converted to utf-8
    error = Decode(encoding, framedata+1, framesize-1, utf8text, bufferlimit, NULL);
    if(error)
        return error;

    // free data
    free(framedata);

    return ID3V2ERROR_NOERROR;
}

//////////////////////////////////////////////////////////////////////////////

int ID3V2_SetTextFrame(ID3V2 *id3v2, unsigned int ID, const char *utf8text, unsigned char encoding)
{
    if(utf8text == NULL)
        return ID3V2ERROR_NOERROR;

    int    error;
    size_t textlength      = strlen(utf8text) + 1; // count '\0' as well
    void  *rawtext;
    size_t rawtextsize;
    size_t textbufferlimit = textlength * 4;       // 4 time the uft-8 encoded size is enough
    unsigned char version  = id3v2->header.version_major;

    // Check version number
    if(encoding == ID3V2TEXTENCODING_UTF16_BE || encoding == ID3V2TEXTENCODING_UTF8)
    {
        if(version == 3)
            return ID3V2ERROR_UNSUPPORTEDENCODING;
    }

    // Allocate memory for encoded text
    rawtext = malloc(textlength * 4);
    if(rawtext == NULL)
    {
        fprintf(stderr, "%s, %i: ", __FILE__, __LINE__);
        fprintf(stderr, "Fatal Error! - malloc returned NULL!\n");
        return ID3V2ERROR_FATAL;
    }

    // Encode text
    //  -1 to not encode '\0' because the ID3v2.3.0 standard does not need it in text frames.
    if(version == 3)
        error = Encode(encoding, utf8text, textlength - 1, rawtext, textbufferlimit, &rawtextsize);
    else
        error = Encode(encoding, utf8text, textlength    , rawtext, textbufferlimit, &rawtextsize);

    if(error)
    {
        free(rawtext);
        return error;
    }

    // Create frame data
    size_t         framesize = rawtextsize + 1;   // text bytes + enc-ID
    unsigned char *framedata = malloc(framesize);
    if(framedata == NULL)
    {
        fprintf(stderr, "%s, %i: ", __FILE__, __LINE__);
        fprintf(stderr, "Fatal Error! - malloc returned NULL!\n");
        free(rawtext);
        return ID3V2ERROR_FATAL;
    }

    framedata[0] = encoding;
    memcpy(framedata + 1, rawtext, rawtextsize);

    // Set Frame
    error = ID3V2_SetFrame(id3v2, ID, framesize, framedata);
    if(error)
    {
        free(rawtext);
        free(framedata);
        return error;
    }

    // done
    free(rawtext);
    free(framedata);

    return ID3V2ERROR_NOERROR;
}

//////////////////////////////////////////////////////////////////////////////

int ID3V2_GetPictureFrame(const ID3V2 *id3v2, unsigned char pictype, 
                          char **mimetype, char **description, void **picture, size_t *picsize)
{
    int error;

    // Get raw data
    size_t         rawsize;
    int            rawoffset = 0;  // tracing offset through the process to know where we are in the frame
    unsigned char *rawdata;
    error = ID3V2_GetFrame(id3v2, 'APIC', &rawsize, (void**)&rawdata);
    if(error)
        return error;

    // Encoding
    unsigned char encoding = rawdata[rawoffset];
    rawoffset++;
    
    // mime-type - ASCII encoded, '\0'-terminated
    size_t mimesize = strlen((char*)&rawdata[rawoffset]);
    char  *mime     = malloc(sizeof(char) * (mimesize + 1)); // +'\0'
    if(mime == NULL)
    {
        fprintf(stderr, "%s, %i: ", __FILE__, __LINE__);
        fprintf(stderr, "Fatal Error! - malloc returned NULL!\n");
        return ID3V2ERROR_FATAL;
    }
    strncpy(mime, (char*)&rawdata[rawoffset], mimesize + 1); // from offset 1 all chars + '\0'
    rawoffset += mimesize + 1; // mimetype-string + '\0'
    
    // picture type
    unsigned char picturetype = rawdata[rawoffset];
    if(picturetype != pictype) // is this the one the user wants to have?
    {
        free(mime);
        fprintf(stderr, "Multiple APIC-frames are not supported yet!\n");
        return ID3V2ERROR_MISFITTINGSUBID;
    }
    rawoffset++; // pictype

    // text description. Spec say raw data: max 64 chars + '\0[\0]' -> max 65*2 bytes = 130bytes
    char *desctext; // utf-8 encoded \0-terminated string
    desctext = malloc(sizeof(char) * ID3V2_MAXPICTUREDESCRIPTIONSIZE * 4); // just allocate four times max length of raw data
    if(desctext == NULL)
    {
        free(mime);
        fprintf(stderr, "%s, %i: ", __FILE__, __LINE__);
        fprintf(stderr, "Fatal Error! - malloc returned NULL!\n");
        return ID3V2ERROR_FATAL;
    }
    
    // Get raw description size
    size_t rawdescsize = 0;
    while(true)
    {
        if(encoding == ID3V2TEXTENCODING_UTF16_BOM || encoding == ID3V2TEXTENCODING_UTF16_BE) // 2 byte encodings
        {
            if(rawdata[rawoffset + rawdescsize] == 0x00 && rawdata[rawoffset + rawdescsize + 1] == 0x00)
            {
                rawdescsize += 2;
                break;
            }
            rawdescsize += 2;
        }
        else // 1 byte encodings
        {
            if(rawdata[rawoffset + rawdescsize] == 0x00)
            {
                rawdescsize += 1;
                break;
            }
            rawdescsize += 1;
        }
    }

    // Decode description
    error = Decode(encoding, &rawdata[rawoffset], rawdescsize, desctext, ID3V2_MAXPICTUREDESCRIPTIONSIZE, NULL);
    if(error)
    {
        free(mime);
        free(desctext);
        return error;
    }
    rawoffset += rawdescsize;

#ifdef DEBUG
    if(encoding == 0x01 || encoding == 0x02) // 2 byte encodings
    {
        printhex(rawdata, 128, 16, 
            0,                            "\e[1;36m", // encoding
            1,                            "\e[1;35m", // mime-type
            1+mimesize+1,                 "\e[1;32m", // pictype
            1+mimesize+1+1,               "\e[1;33m", // BOM
            1+mimesize+1+1+2,             "\e[0;35m", // description
            1+mimesize+1+1+2+rawdescsize, "\e[1;34m", // pic-data
            -1);
    }
    else // 1 byte encodings
    {
        printhex(rawdata, 128, 16, 
            0,                          "\e[1;36m", // encoding
            1,                          "\e[1;35m", // mime-type
            1+mimesize+1,               "\e[1;32m", // pictype
            1+mimesize+1+1,             "\e[0;35m", // description
            1+mimesize+1+1+rawdescsize, "\e[1;34m", // pic-data
            -1);
    }
    printf("\n");
#endif

    // now just copy the picture into a buffer
    unsigned char *picdata;
    unsigned int   picdatasize = rawsize-rawoffset;
    picdata = malloc(picdatasize);
    if(picdata == NULL)
    {
        free(mime);
        free(desctext);
        fprintf(stderr, "%s, %i: ", __FILE__, __LINE__);
        fprintf(stderr, "Fatal Error! - malloc returned NULL!\n");
        return ID3V2ERROR_FATAL;
    }
    memcpy(picdata, rawdata+rawoffset, picdatasize);
    

    if(mimetype != NULL)
        *mimetype = mime;
    else
        free(mime);

    if(description != NULL)
        *description = desctext;
    else
        free(desctext);

    if(picture != NULL)
        *picture = picdata;
    else
        free(picdata);

    if(picsize != NULL)
        *picsize = picdatasize;

    free(rawdata);
    return ID3V2ERROR_NOERROR;
}

//////////////////////////////////////////////////////////////////////////////

int ID3V2_SetPictureFrame(ID3V2 *id3v2, unsigned char pictype, 
                          const char *mimetype, const char *description, unsigned char encoding,
                          const void *picture, size_t picsize)
{
    int error;

    // Check if encoding is supported
    if(encoding == ID3V2TEXTENCODING_UTF16_BE || encoding == ID3V2TEXTENCODING_UTF8)
    {
        if(id3v2->header.version_major == 3)
            return ID3V2ERROR_UNSUPPORTEDENCODING;
    }

    // Encode description
    char   rawdescdata[ID3V2_MAXPICTUREDESCRIPTIONSIZE];
    size_t rawdescsize;

    if(description != NULL)
    {
        error = Encode(encoding, (char*)description, strlen(description)+1, 
                       rawdescdata, ID3V2_MAXPICTUREDESCRIPTIONSIZE, &rawdescsize);
        if(error)
            return error;
    }
    else // Create empty description if no one is given
    {
        switch(encoding)
        {
            case ID3V2TEXTENCODING_UTF16_BOM:
                rawdescdata[0] = 0xFF;
                rawdescdata[1] = 0xFE;
                rawdescdata[2] = 0x00;
                rawdescdata[3] = 0x00;
                rawdescsize    = 4;
                break;

            case ID3V2TEXTENCODING_UTF16_BE:
                rawdescdata[0] = 0x00;
                rawdescdata[1] = 0x00;
                rawdescsize    = 2;
                break;

            case ID3V2TEXTENCODING_UTF8:
            case ID3V2TEXTENCODING_ISO8859_1:
                rawdescdata[0] = 0x00;
                rawdescsize    = 1;
                break;

            default:
                return ID3V2ERROR_UNSUPPORTEDENCODING;
        }
    }

    // Build frame
    size_t         framesize   = 0;
    unsigned char *framedata   = NULL;
    unsigned int   frameoffset = 0;

    // Calculate frame size
    framesize += 1;                     // encoding
    framesize += strlen(mimetype) + 1;  // mimetype + '\0'
    framesize += 1;                     // picture type
    framesize += rawdescsize;           // description
    framesize += picsize;               // picture

    // Create frame data
    framedata = malloc(framesize);
    if(framedata == NULL)
    {
        fprintf(stderr, "%s, %i: ", __FILE__, __LINE__);
        fprintf(stderr, "Fatal Error! - malloc returned NULL!\n");
        return ID3V2ERROR_FATAL;
    }

    framedata[frameoffset++] = encoding;                    // encoding: UTF-16
    strcpy((char*)(framedata+frameoffset), mimetype);       // \_ mime type
    frameoffset += strlen(mimetype)+1;                      // /
    framedata[frameoffset++] = pictype;                     // picture type (should be 0x03 - front cover)
    memcpy(framedata+frameoffset, rawdescdata, rawdescsize);// \_ encoded description
    frameoffset += rawdescsize;                             // /
    memcpy(framedata+frameoffset, picture, picsize);        // picture data

#ifdef DEBUG
    size_t mimesize = strlen(mimetype);
    printhex(framedata, 128, 16, 
            0,                            "\e[1;36m", // encoding
            1,                            "\e[1;35m", // mime type
            1+mimesize+1,                 "\e[1;32m", // picture type
            1+mimesize+1+1,               "\e[1;33m", // Byte Order Mark (BOM) when UTF-16 encoded
            1+mimesize+1+1+2,             "\e[0;35m", // description
            1+mimesize+1+1+2+rawdescsize, "\e[1;34m", // picture data
            -1);
    printf("\n");
#endif
    // Set frame
    error = ID3V2_SetFrame(id3v2, 'APIC', framesize, framedata);
    if(error)
    {
        free(framedata);
        return error;
    }

    // done
    free(framedata);
    return ID3V2ERROR_NOERROR;
}
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

