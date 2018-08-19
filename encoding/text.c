#include <errno.h> 
#include <stdio.h>
#include <stdlib.h>
#include <id3v2.h>
#include <id3v2frame.h>
#include <iconv.h>
#include <encoding/text.h>

#ifdef DEBUG
#include <printhex.h>
#endif

int Transcode(const char *from, const char *to, const void *input, size_t inputbytelimit, void *output, size_t outputbytelimit, size_t *actualsize)
{
    iconv_t cd;
    size_t  retval;

    cd = iconv_open(to, from); 
    if(cd == (iconv_t) -1)
    {
        if(errno == EINVAL)
            fprintf(stderr, "\e[1;31miconv_open failed with error \e[1;35mEINVAL\e[1;31m!\e[0m\n");
        else
            fprintf(stderr, "\e[1;31miconv_open failed with unexpected error \e[1;35merrno = %i\e[1;31m!\e[0m\n", errno);
        return ID3V2ERROR_UNICODEERROR;
    }

    size_t remainingbytes = outputbytelimit;
    retval = iconv(cd, (char**)&input, &inputbytelimit, (char**)&output, &remainingbytes);
    if(retval == (size_t) -1)
    {
        if(errno == E2BIG)
            fprintf(stderr, "\e[1;31miconv failed with error \e[1;35mE2BIG\e[1;31m!\e[0m\n");
        else if(errno == EILSEQ)
            fprintf(stderr, "\e[1;31miconv failed with error \e[1;35mEILSEQ\e[1;31m!\e[0m\n");
        else if(errno == EINVAL)
            fprintf(stderr, "\e[1;31miconv failed with error \e[1;35mEINVAL\e[1;31m!\e[0m\n");
        else
            fprintf(stderr, "\e[1;31miconv failed with unexpected error \e[1;35merrno = %i\e[1;31m!\e[0m\n", errno);
        return ID3V2ERROR_UNICODEERROR;
    }

    iconv_close(cd);

    if(actualsize != NULL)
        *actualsize = outputbytelimit - remainingbytes;
    return ID3V2ERROR_NOERROR;
}



const char* ID3v2CodeID2String(unsigned char id3v2encoding)
{
    switch(id3v2encoding)
    {
        case ID3V2TEXTENCODING_ISO8859_1:
            return "ISO8859-1";
        case ID3V2TEXTENCODING_UTF16_BOM:
            return "UTF-16";
        case ID3V2TEXTENCODING_UTF16_BE:
            return "UTF-16BE";
        case ID3V2TEXTENCODING_UTF8:
            return "UTF-8";
        default:
            fprintf(stderr, "\e[1;31mText encoding 0x%X not specified in ID3v2.3.0 or ID3v2.4.0 standard!\e[0m\n", id3v2encoding);
            return NULL;
    }
}


int Decode(unsigned char id3v2encoding, void *rawdata,  size_t rawdatasize, char *utf8text, size_t textlengthlimit, size_t *actualsize)
{
    // prepare transcoding
    const char *code;
    code = ID3v2CodeID2String(id3v2encoding);
    if(code == NULL)
        return ID3V2ERROR_UNSUPPORTEDENCODING;

    // transcode
    size_t actuallength = 0;    // Bytes written into the output buffer
    int error;
    error = Transcode(code, "UTF-8", rawdata, rawdatasize, utf8text, textlengthlimit, &actuallength);
    if(error)
        return error;

    // make sure string is terminated - not all sources provide one, so iconv may not set one.
    if(utf8text[actuallength-1] != '\0')
    {
        if(actuallength >= textlengthlimit)
            return ID3V2ERROR_BUFFEROVERFLOW;

        utf8text[actuallength] = '\0';
        actuallength++;
    }

    if(actualsize != NULL)
        *actualsize = actuallength;

    return ID3V2ERROR_NOERROR;
}


int Encode(unsigned char id3v2encoding, const char *utf8text, size_t textlength,  void *rawdata,  size_t rawdatasizelimit, size_t *actualsize)
{
    // prepare transcoding
    const char *code;
    code = ID3v2CodeID2String(id3v2encoding);
    if(code == NULL)
        return ID3V2ERROR_UNSUPPORTEDENCODING;

    // transcode
    size_t actuallength = 0;    // Bytes written into the output buffer
    int error;
    error = Transcode("UTF-8", code, utf8text, textlength, rawdata, rawdatasizelimit, &actuallength);
    if(error)
        return error;

    if(actualsize != NULL)
        *actualsize = actuallength;

    return ID3V2ERROR_NOERROR;
}


// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

