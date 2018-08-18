#include <stdlib.h>
#include <endian.h>
#include <encoding/crc.h>


void ID3V2_EncodeCRC(unsigned long  crc,       unsigned char enccrc[5])
{
    if(enccrc == NULL)
        return;

    // 1.: LE -> BE
    unsigned long becrc;
    becrc = htobe32(crc);

    // 1.: xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    //  -> 0000xxxx0xxxxxxx0xxxxxxx0xxxxxxx0xxxxxxx
    unsigned int byte = 0;
    for(int i=0; i<5; i++)
    {
        byte      = becrc >> ((4-i)*7);
        byte     &= 0x7F;
        enccrc[i] = byte;
    }

    return;
}

void ID3V2_DecodeCRC(unsigned long *crc, const unsigned char enccrc[5])
{
    if(crc == NULL || enccrc == NULL)
        return;

    // 1.: 0000xxxx0xxxxxxx0xxxxxxx0xxxxxxx0xxxxxxx
    //  -> xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    unsigned int  byte = 0;
    unsigned long tmp  = 0;
    for(int i=0; i<5; i++)
    {
        byte  = enccrc[i];
        byte &= 0x7F;
        tmp  |= byte << ((4-i)*7);
    }

    // 2.: BE -> LE
    *crc = be32toh(tmp);

    return;
}

// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

