#include <errno.h> 
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <endian.h>
#include <stdbool.h>
#include <encoding/size.h>
#include <id3v2.h>


int ID3V240_ReadExtendedHeader(ID3V2 *id3)
{
    unsigned int bigendian;

    unsigned int encsize; 
    fread(&encsize, 4, 1, id3->file);
    id3->extheader.size   = ID3V2_DecodeSize(encsize);
    id3->header.realsize += id3->extheader.size;

    unsigned char numofflagbytes;
    fread(&numofflagbytes, 1, 1, id3->file);
    if(numofflagbytes != 1)
    {
        fprintf(stderr, "Number of flag bytes not 1 as specified! (actually %i)\n", numofflagbytes);
        free(id3->path);
        free(id3);
        return ID3V2ERROR_NOTSUPPORTED;
    }

    unsigned char flags;
    fread(&flags, 1, 1, id3->file);

    // Parse flags and read its attached data
    unsigned char flagdatasize;

    // Updated flag
    if(flags & ID3V240EXTHDRFLAG_UPDATE)
    {
        id3->extheader.flag_update = true;

        fread(&flagdatasize, 1, 1, id3->file); // This byte is always 0
        if(flagdatasize != 0)
        {
            fprintf(stderr, "Attached data for Updated-Flag is not 0 as specified! (actually %i)\n", flagdatasize);
            return ID3V2ERROR_NOTSUPPORTED;
        }
    }
    else
        id3->extheader.flag_update = false;

    // CRC flag
    if(flags & ID3V240EXTHDRFLAG_CRC)
    {
        id3->extheader.flag_crc = true;

        fread(&flagdatasize, 1, 1, id3->file); // This byte is always 5
        if(flagdatasize != 5)
        {
            fprintf(stderr, "Attached data for CRC-Flag is not 5 as specified! (actually %i)\n", flagdatasize);
            return ID3V2ERROR_NOTSUPPORTED;
        }
        unsigned char enccrc[5]; // encoded CRC: 7bit/byte
        fread(enccrc, 1, 5, id3->file);
        // TODO: decode CRC value
    }
    else
    {
        id3->extheader.flag_crc = false;
        id3->extheader.crc      = 0x00000000;
    }

    // Restrictions flag
    if(flags & ID3V240EXTHDRFLAG_RESTRICTED)
    {
        id3->extheader.flag_restricted = true;

        fread(&flagdatasize, 1, 1, id3->file); // This byte is always 1
        if(flagdatasize != 1)
        {
            fprintf(stderr, "Attached data for CRC-Flag is not 1 as specified! (actually %i)\n", flagdatasize);
            return ID3V2ERROR_NOTSUPPORTED;
        }
        unsigned char restrictions;
        fread(&restrictions, 1, 5, id3->file);
        id3->extheader.restrictions = restrictions;
    }
    else
    {
        id3->extheader.flag_restricted = false;
        id3->extheader.restrictions    = 0x00;
    }

    return ID3V2ERROR_NOERROR;
}



int ID3V230_ReadExtendedHeader(ID3V2 *id3)
{
    unsigned int   bigendian;
    unsigned short flags;
    fread(&bigendian, 4, 1, id3->file); id3->extheader.size        = be32toh(bigendian);
    fread(&bigendian, 2, 1, id3->file); flags                      = be16toh(bigendian);
    fread(&bigendian, 4, 1, id3->file); id3->extheader.paddingsize = be32toh(bigendian);

    id3->header.realsize += id3->extheader.size + 4; 
    // +4: Size value of the ext header is not included in extheader.size

    // Initialize ID3v2.4.0 flags that are not used in ID3v2.3.0
    id3->extheader.flag_update     = false;
    id3->extheader.flag_restricted = false;
    id3->extheader.restrictions    = 0x00;

    // Process ID3v2.3.0 flags
    if(flags & ID3V230EXTHDRFLAG_CRC)
    {
        fread(&bigendian, 4, 1, id3->file);
        id3->extheader.flag_crc = true;
        id3->extheader.crc      = be32toh(bigendian);
    }
    else
    {
        id3->extheader.flag_crc = true;
        id3->extheader.crc      = 0x00000000;
    }

    return ID3V2ERROR_NOERROR;
}

// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

