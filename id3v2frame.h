#ifndef ID3V2FRAME_H
#define ID3V2FRAME_H

#define ID3V2TEXTENCODING_ISO8859_1 0x00    /* 0x20 … 0xFF */
#define ID3V2TEXTENCODING_UTF16_BOM 0x01    /* utf-16 starting with an byte order mark */
#define ID3V2TEXTENCODING_UTF16_BE  0x02    /* utf-16 big endian (ID3v2.4.0) */
#define ID3V2TEXTENCODING_UTF8      0x03    /* utf-8             (ID3v2.4.0) */

#define ID3V2_GetTitle(i,t,l)  ID3V2_GetTextFrame(i, 'TIT2', t, l)
#define ID3V2_GetAlbum(i,t,l)  ID3V2_GetTextFrame(i, 'TALB', t, l)
#define ID3V2_GetArtist(i,t,l) ID3V2_GetTextFrame(i, 'TPE1', t, l)
#define ID3V2_GetYear(i,t,l)   ID3V2_GetTextFrame(i, 'TYER', t, l)
#define ID3V2_GetTrack(i,t,l)  ID3V2_GetTextFrame(i, 'TRCK', t, l)
#define ID3V2_GetCD(i,t,l)     ID3V2_GetTextFrame(i, 'TPOS', t, l)

/*
 * ID: 4 Byte Frame ID
 * utf8text: a buffer where the decoded text frame text is stored at
 *           The text will be a 0-terminated utf-8 encoded string.
 *           When utf8text is NULL, then the function returns without any error
 *           after getting the addressed Frame.
 * bufferlimit: is the size of the utf8text buffer in bytes
 */
int ID3V2_GetTextFrame(ID3V2 *id3v2, const unsigned int ID, char *utf8text, size_t bufferlimit);



#define ID3V2_SetTitle(i,t)  ID3V2_SetTextFrame(i, 'TIT2', t, ID3V2TEXTENCODING_UTF16_BOM)
#define ID3V2_SetAlbum(i,t)  ID3V2_SetTextFrame(i, 'TALB', t, ID3V2TEXTENCODING_UTF16_BOM)
#define ID3V2_SetArtist(i,t) ID3V2_SetTextFrame(i, 'TPE1', t, ID3V2TEXTENCODING_UTF16_BOM)
#define ID3V2_SetYear(i,t)   ID3V2_SetTextFrame(i, 'TYER', t, ID3V2TEXTENCODING_UTF16_BOM)
#define ID3V2_SetTrack(i,t)  ID3V2_SetTextFrame(i, 'TRCK', t, ID3V2TEXTENCODING_UTF16_BOM)
#define ID3V2_SetCD(i,t)     ID3V2_SetTextFrame(i, 'TPOS', t, ID3V2TEXTENCODING_UTF16_BOM)

/*
 * ID: 4 Byte Frame ID
 * utf8text: 0-terminated utf-8 encoded string that shall be stored in a Text Frame
 * encoding: The ID3V2TEXTENCODING to use to encode the text
 *
 * !! When an encoding is used that is not supported by the given ID3 version, the version gets updated in the ID3V2 struct
 */
int ID3V2_SetTextFrame(ID3V2 *id3v2, const unsigned int ID, char *utf8text, unsigned char encoding);

#define ID3V2_MAXPICTUREDESCRIPTIONSIZE ((64+1)*2) /*max 64 Chars + termination character UTF-16 encoded*/
/*
 * mimetype, description and picture memory must be feed by the caller when not NULL
 * description: can be NULL
 * encoding: The ID3V2TEXTENCODING to use to encode the text
 */
int ID3V2_GetPictureFrame(ID3V2 *id3v2, const unsigned char pictype, 
                          char **mimetype, char **description, void **picture, size_t *picsize);

int ID3V2_SetPictureFrame(ID3V2 *id3v2, const unsigned char pictype, 
                          const char *mimetype, const char *description, unsigned char encoding,
                          void *picture, size_t picsize);
#endif



// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

