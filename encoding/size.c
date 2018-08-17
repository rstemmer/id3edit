#include <errno.h> 
#include <endian.h>
#include <encoding/size.h>


unsigned int ID3V2_EncodeSize(unsigned int size)
{
    // 1.: 0000xxxxxxxxxxxxxxxxxxxxxxxxxxxx
    //  -> 0xxxxxxx0xxxxxxx0xxxxxxx0xxxxxxx
    unsigned int byte = 0;
    unsigned int tmp  = 0;
    for(int i=0; i<4; i++)
    {
        byte  = size >> (i*7);
        byte &= 0x7F;
        tmp  |= byte << (i*8);
    }

    // 2.: LE -> BE
    unsigned int encsize;
    encsize = htobe32(tmp);

    return encsize;
}

unsigned int ID3V2_DecodeSize(unsigned int encsize)
{
    // 1.: BE -> LE
    unsigned int tmp;
    tmp = be32toh(encsize);

    // 2.: 0xxxxxxx0xxxxxxx0xxxxxxx0xxxxxxx
    //  -> 0000xxxxxxxxxxxxxxxxxxxxxxxxxxxx
    unsigned int byte = 0;
    unsigned int size = 0;
    for(int i=0; i<4; i++)
    {
        byte  = tmp  >> (i*8);
        byte &= 0x7F;
        size |= byte << (i*7);
    }

    return size;
}

// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

