#include <errno.h> 
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <endian.h>
#include <stdbool.h>
#include <encoding/size.h>
#include <encoding/crc.h>
#include <crc32.h>
#include <id3v2.h>

int ID3V2_UpdateExtendedHeader(ID3V2 *id3, bool update, bool crc, unsigned char restrictions)
{
    unsigned char version = id3->header.version_major;

    // First check if a new header must be created
    if(id3->extheader.size == 0)
    {
        unsigned int headersize;

        if(version == 4)
        {
            id3->extheader.size   = 6;
            id3->header.realsize +=  6;
        }
        else
        {
            id3->extheader.size        = 6;
            id3->extheader.paddingsize = ID3V2PADDING;
            id3->header.realsize      += 10;
        }

        id3->header.flags |= ID3V2HEADERFLAG_EXTENDEDHEADER;
    }

    // Now update for each flag
    if(update && version == 4)
    {
        id3->extheader.flag_update = true;
        id3->extheader.size       += 1;
        id3->header.realsize      += 1;
    }
    if(crc)
    {
        id3->extheader.flag_crc = true;
        if(version == 4)
        {
            id3->extheader.size  += 6; // 5 Byte CRC + data length byte
            id3->header.realsize += 6;
        }
        else
        {
            id3->extheader.size  += 4;
            id3->header.realsize += 4;
        }
    }
    if(restrictions && version == 4)
    {
        id3->extheader.flag_restricted = true;
        id3->extheader.size           += 2;
        id3->header.realsize          += 2;
    }

    return ID3V2ERROR_NOERROR;
}

//////////////////////////////////////////////////////////////////////////////

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
        ID3V2_DecodeCRC(&id3->extheader.crc, enccrc);
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
    unsigned long  bigendian;
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

//////////////////////////////////////////////////////////////////////////////
unsigned long crcdataposition = 0;  // The position where to update the CRC dummy data (read code below)

int ID3V240_WriteExtendedHeader(const ID3V2 *id3, FILE *file)
{
    // Write encoded header size
    unsigned long encsize;
    encsize = ID3V2_EncodeSize(id3->extheader.size);
    fwrite(&encsize, 4, 1, file);

    // Number of flag bytes (always 1)
    fputc(0x01, file);

    // Write flags
    unsigned char flags = 0x00;
    if(id3->extheader.flag_update)
        flags |= ID3V240EXTHDRFLAG_UPDATE;
    if(id3->extheader.flag_crc)
        flags |= ID3V240EXTHDRFLAG_CRC;
    if(id3->extheader.flag_restricted)
        flags |= ID3V240EXTHDRFLAG_RESTRICTED;

    fwrite(&flags, 1, 1, file);

    // Write flag data
    if(id3->extheader.flag_update)
    {
        fputc(0x00, file); // flag data length (always 0 for this flag)
    }
    if(id3->extheader.flag_crc)
    {
        fputc(0x05, file); // flag data length (always 5 for this flag)
        // The CRC32 sum can only be calculated after writing the tag.
        // Therefore only dummy data will be written right now.
        // Later ID3V2_UpdateCRC must be called to replace the dummy data
        crcdataposition = ftell(file);
        fputc(0x00, file); // dummy data
        fputc(0x00, file); // dummy data
        fputc(0x00, file); // dummy data
        fputc(0x00, file); // dummy data
        fputc(0x00, file); // dummy data
    }
    if(id3->extheader.flag_restricted)
    {
        fputc(0x01, file); // flag data length (always 1 for this flag)
        fputc(id3->extheader.restrictions, file);
    }

    return ID3V2ERROR_NOERROR;
}



int ID3V230_WriteExtendedHeader(const ID3V2 *id3, FILE *file)
{
    // Write size
    unsigned long bigendian;
    bigendian = htobe32(id3->extheader.size);
    fwrite(&bigendian, 4, 1, file);

    // Write flags
    if(id3->extheader.flag_crc)
        fputc(0x80, file);
    else
        fputc(0x00, file);
    fputc(0x00, file);

    // Write padding size
    bigendian = htobe32(ID3V2PADDING);
    fwrite(&bigendian, 4, 1, file);

    // When there is a CRC check sum, save position and write dummy data
    if(id3->extheader.flag_crc)
    {
        crcdataposition = ftell(file);
        fputc(0x00, file); // dummy data
        fputc(0x00, file); // dummy data
        fputc(0x00, file); // dummy data
        fputc(0x00, file); // dummy data
    }

    return ID3V2ERROR_NOERROR;
}

//////////////////////////////////////////////////////////////////////////////

int ID3V240_UpdateCRC(const ID3V2 *id3, FILE *file)
{
    // For ID3v2.4.0, checksum is of frames and padding
    long currentpos;
    long startofdata;
    long endofdata;
    currentpos  = ftell(file);              // remember current position in file
    startofdata = 10+id3->extheader.size;   // The data to consider start after the extended header
    endofdata   = 10+id3->header.realsize + ID3V2PADDING;  // 10: Size of tag header

    // Calculate CRC sum
    int error;
    unsigned long crc;
    error = CRC32FromFile(file, startofdata, endofdata, &crc);
    if(error)
    {
        fprintf(stderr, "Calculating CRC sum failed. File destroyed, sorry :)");
        return ID3V2ERROR_FATAL;
    }

    // Encode and write
    unsigned char enccrc[5];
    ID3V2_EncodeCRC(crc, enccrc);
    fseek(file, crcdataposition, SEEK_SET);
    fwrite(enccrc, 1, 5, file);

    // Return to position in file before changing the checksum
    fseek(file, currentpos, SEEK_SET);

    return ID3V2ERROR_NOERROR;
}



int ID3V230_UpdateCRC(const ID3V2 *id3, FILE *file)
{
    // For ID3v2.3.0, checksum is of frames only - keep in mind that the ext header size does not include itself
    long currentpos;
    long startofdata;
    long endofdata;
    currentpos  = ftell(file);              // remember current position in file
    startofdata = 10+id3->extheader.size+4; // The data to consider start after the extended header
    endofdata   = 10+id3->header.realsize;  // 10: Size of tag header

    // Calculate CRC sum
    int error;
    unsigned long crc;
    error = CRC32FromFile(file, startofdata, endofdata, &crc);
    if(error)
    {
        fprintf(stderr, "Calculating CRC sum failed. File destroyed, sorry :)");
        return ID3V2ERROR_FATAL;
    }

    // Encode and write
    unsigned long enccrc;
    enccrc = htobe32(crc);
    fseek(file, crcdataposition, SEEK_SET);
    fwrite(&enccrc, 4, 1, file);

    // Return to position in file before changing the checksum
    fseek(file, currentpos, SEEK_SET);

    return ID3V2ERROR_NOERROR;
}

// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

