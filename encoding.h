#ifndef UTFX_H
#define UTFX_H


#define UTF16BOM_LE 0xFEFF
#define UTF16BOM_BE 0xFFFE


/*
 *  first char is the BOM when not UTF-16BE/LE is specified
 *  Terminating character will be "transcoded" as well
 *  actualsize is the number of bytes written into the output buffer
 */
int Transcode(const char *from, const char *to, void *input, size_t inputbytelimit, void *output, size_t outputbytelimit, size_t *actualsize);

/*
 * returns a 0-terminated utf-8 string in utf8text variable
 */
int Decode(unsigned char id3v2encoding, void *rawdata,  size_t rawdatasize, char *utf8text, size_t textlengthlimit);
int Encode(unsigned char id3v2encoding, char *utf8text, size_t textlength,  void *rawdata,  size_t rawdatasizelimit);
#endif



// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

