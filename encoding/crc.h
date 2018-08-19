#ifndef ENCODING_CRC_H
#define ENCODING_CRC_H


void ID3V2_EncodeCRC(unsigned long  crc,       unsigned char enccrc[5]);
void ID3V2_DecodeCRC(unsigned long *crc, const unsigned char enccrc[5]);


#endif


// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

