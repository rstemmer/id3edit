#ifndef ID3V2FRAME_H
#define ID3V2FRAME_H

#define ID3V2TEXTENCODING_ISO8859_1 0x00    /* 0x20 … 0xFF */
#define ID3V2TEXTENCODING_UTF16_BOM 0x01    /* utf-16 starting with an byte order mark */
#define ID3V2TEXTENCODING_UTF16_BE  0x02    /* utf-16 big endian (ID3v2.4.0) */
#define ID3V2TEXTENCODING_UTF8      0x03    /* utf-8             (ID3v2.4.0) */


/*
 * ID: 4 Byte Frame ID
 * utf8text: a buffer where the decoded text frame text is stored at
 *           The text will be a 0-terminated utf-8 encoded string.
 *           When utf8text is NULL, then the function returns without any error
 *           after getting the addressed Frame.
 * bufferlimit: is the size of the utf8text buffer in bytes
 */
int ID3V2_GetTextFrame(const ID3V2 *id3v2, unsigned int ID, char *utf8text, size_t bufferlimit);

/*
 * ID: 4 Byte Frame ID
 * utf8text: 0-terminated utf-8 encoded string that shall be stored in a Text Frame
 * encoding: The ID3V2TEXTENCODING to use to encode the text
 *
 * !! When an encoding is used that is not supported by the given ID3 version, the version gets updated in the ID3V2 struct
 */
int ID3V2_SetTextFrame(ID3V2 *id3v2, unsigned int ID, const char *utf8text, unsigned char encoding);

#define ID3V2_MAXPICTUREDESCRIPTIONSIZE ((64+1)*2) /*max 64 Chars + termination character UTF-16 encoded*/
/*
 * mimetype, description and picture memory must be feed by the caller when not NULL
 * description: can be NULL
 * encoding: The ID3V2TEXTENCODING to use to encode the text
 */
int ID3V2_GetPictureFrame(const ID3V2 *id3v2, unsigned char pictype, 
                          char **mimetype, char **description, void **picture, size_t *picsize);

int ID3V2_SetPictureFrame(ID3V2 *id3v2, unsigned char pictype, 
                          const char *mimetype, const char *description, unsigned char encoding,
                          const void *picture, size_t picsize);
#endif



// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

