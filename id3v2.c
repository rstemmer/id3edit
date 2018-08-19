#include <errno.h> 
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>
#include <id3v2.h>
#include <stdbool.h>
#include <encoding/text.h>
#include <encoding/size.h>
#include <crc32.h>


bool OPT_PrintHeader = false;

int ID3V2_Open(ID3V2 **id3v2, const char *path, bool createtag)
{
    unsigned int bigendian;
    *id3v2 = NULL; // no undefined state please - the new struct gets assigned to id3v2 at the end, if everything if fine

    // Create main structure
    ID3V2 *id3 = (ID3V2*) malloc(sizeof(ID3V2));
    if(id3 == NULL)
    {
        fprintf(stderr, "%s, %i: ", __FILE__, __LINE__);
        fprintf(stderr, "Fatal Error! - malloc returned NULL!\n");
        return ID3V2ERROR_FATAL;
    }

    // Copy path
    size_t pathlength = strlen(path) + 1;  // don't forget the terminating \0
    id3->path = (char*) malloc(sizeof(char)*pathlength);
    if(id3->path == NULL)
    {
        fprintf(stderr, "%s, %i: ", __FILE__, __LINE__);
        fprintf(stderr, "Fatal Error! - malloc returned NULL!\n");
        free(id3);
        return ID3V2ERROR_FATAL;
    }

    strncpy(id3->path, path, pathlength);

    // Open file
    id3->file = fopen(path, "rb");
    if(id3->file == NULL)
    {
        fprintf(stderr, "Opening \"%s\" failed with error \"%s\"\n", path, strerror(errno));
        free(id3->path);
        free(id3);
        return ID3V2ERROR_PATHERROR;
    }

    // Read Header
    fread(&id3->header.ID,            1, 3, id3->file);
    fread(&id3->header.version_major, 1, 1, id3->file);
    fread(&id3->header.version_minor, 1, 1, id3->file);
    fread(&id3->header.flags,         1, 1, id3->file);

    // the header size is stored as big endian. This must be converted to little endian.
    // beside this, it has 7 bit per byte...
    unsigned int encsize; 
    fread(&encsize, 4, 1, id3->file);
    id3->header.origsize = ID3V2_DecodeSize(encsize);
    id3->header.realsize = 0;   // initialize with zero. From now, for every byte read this will be incremented

    // print the header if set in the options, or if debugging is enabled
#ifndef DEBUG
    if(OPT_PrintHeader)
#endif
    {
        printf("\e[1;37mHeader\n");
        printf("\e[1;34mID:      \033[0;36m\'%c%c%c\'\n", id3->header.ID[0], id3->header.ID[1], id3->header.ID[2]);
        printf("\e[1;34mVersion: \033[0;36m2.%i.%i\n",    id3->header.version_major, id3->header.version_minor);
        printf("\e[1;34mFlags:   \033[0;36m0x%02X\n",     id3->header.flags);
        printf("\e[1;34mSize:    \033[0;36m%i\n",         id3->header.origsize);
        if(createtag)
            printf("\e[1;30m\tCreating new Tag if 0x%02X == 0xFF && 0x%02X ∈ {0xFD,0xFB,0xFA,0xF3}\n", id3->header.ID[0], id3->header.ID[1]);
    }

    // check if I support this ID3 thing. If not, create one if allowed
    if(id3->header.ID[0] != 'I' || id3->header.ID[1] != 'D' || id3->header.ID[2] != '3')
    {
        // in case this is a bare mp3 file, just create a new tag
        if( (createtag && id3->header.ID[0] == 0xFF && id3->header.ID[1] == 0xFB)   // *.mp3 (standard)
         || (createtag && id3->header.ID[0] == 0xFF && id3->header.ID[1] == 0xFA)   // *.mp3 (found somewhere)
         || (createtag && id3->header.ID[0] == 0xFF && id3->header.ID[1] == 0xF3)   // *.mp3 (found somewhere)
         || (createtag && id3->header.ID[0] == 0xFF && id3->header.ID[1] == 0xFD) ) // *.mp3 (found somewhere)
        {
            id3->header.ID[0]         = 'I';
            id3->header.ID[1]         = 'D';
            id3->header.ID[2]         = '3';
            id3->header.version_major = 3;      // \_ version 2.3.0
            id3->header.version_minor = 0;      // /
            id3->header.flags         = 0;      // No flags
            id3->header.origsize      = 0;      // No frames
            id3->rawmp3               = true;   // there was no ID3 Tag before
            // Reverse previous attempts to read the ID3 header
            fseek(id3->file, 0, SEEK_SET);  
        }
        else
        {
            fprintf(stderr, "ID: \'%c%c%c\' (%02X%02X%02X) not supported!\n", id3->header.ID[0], 
                                                                              id3->header.ID[1], 
                                                                              id3->header.ID[2],
                                                                              id3->header.ID[0], 
                                                                              id3->header.ID[1], 
                                                                              id3->header.ID[2]);
            free(id3->path);
            free(id3);
            return ID3V2ERROR_NOTSUPPORTED;
        }
    }
    else
    {
        id3->rawmp3 = false; // there is a valid ID3 tag in the source file
    }

    // Check version
    if((id3->header.version_major != 3 && id3->header.version_major != 4) || id3->header.version_minor != 0)
    {
        fprintf(stderr, "Version 2.%i.%i not supported! (just 2.3.0 and 2.4.0 allowed)\n",
                id3->header.version_major, 
                id3->header.version_minor);
        free(id3->path);
        free(id3);
        return ID3V2ERROR_NOTSUPPORTED;
    }

    // Check flags
    if(id3->header.flags != 0)
    {
#ifndef DEBUG
        if(OPT_PrintHeader)
#endif
        {
            printf("\e[1;37mFlags              Set Support\n");
            printf("\e[1;34mUnsynchronisation  %s \e[1;34m[\e[1;31m✘\e[1;34m]\e[0m\n", 
                    (id3->header.flags & ID3V2HEADERFLAG_UNSYNCHRONISATION)?
                    "\e[1;34m[\e[1;36m✔\e[1;34m]":
                    "\e[1;34m[\e[1;30m✘\e[1;34m]");
            printf("\e[1;34mExtended Header    %s \e[1;34m[\e[1;33m✔\e[1;34m] \e[0;33m(partially)\e[0m\n",
                    (id3->header.flags & ID3V2HEADERFLAG_EXTENDEDHEADER)?
                    "\e[1;34m[\e[1;36m✔\e[1;34m]":
                    "\e[1;34m[\e[1;30m✘\e[1;34m]");
            printf("\e[1;34mExperimantal Tag   %s \e[1;34m[\e[1;32m✔\e[1;34m]\e[0m\n", 
                    (id3->header.flags & ID3V2HEADERFLAG_EXPERIMENTAL)?
                    "\e[1;34m[\e[1;36m✔\e[1;34m]":
                    "\e[1;34m[\e[1;30m✘\e[1;34m]");
            printf("\e[1;34mFooter available   %s \e[1;34m[\e[1;31m✘\e[1;34m] \e[1;30m(ID3v2.4.0 only)\e[0m\n", 
                    (id3->header.flags & ID3V2HEADERFLAG_FOOTER)?
                    "\e[1;34m[\e[1;36m✔\e[1;34m]":
                    "\e[1;34m[\e[1;30m✘\e[1;34m]");
        }

        // Some flags are not supported. Exit id3edit when they are set.
        if(id3->header.flags & (ID3V2HEADERFLAG_UNSYNCHRONISATION | ID3V2HEADERFLAG_FOOTER))
        {
            fprintf(stderr, "Unsupported flags detected. Set flags are: 0x%02X\n", id3->header.flags);
            free(id3->path);
            free(id3);
            return ID3V2ERROR_NOTSUPPORTED;
        }
    }

    // read extended header if available
    if(id3->header.flags & ID3V2HEADERFLAG_EXTENDEDHEADER)
    {
        int error;
        if(id3->header.version_major == 4) // ID3v2.4.0 uses a different structure
            error = ID3V240_ReadExtendedHeader(id3);
        else
            error = ID3V230_ReadExtendedHeader(id3);

        if(error)
        {
            free(id3->path);
            free(id3);
            return error;
        }
#ifndef DEBUG
        if(OPT_PrintHeader)
#endif
        {
            printf("\e[1;37mExtended Header\n");
            printf("\e[1;34mSize:         \033[0;36m%i\n", id3->extheader.size);
            printf("\e[1;34mPadding Size: \033[0;36m%i\n", id3->extheader.paddingsize);
            printf("\e[1;37mExtended Flags   Set Support\n");
            printf("\e[1;34mUpdated          %s \e[1;34m[\e[1;31m✘\e[1;34m] \e[0;33m(Will be ignored)\e[0m\n", 
                    (id3->extheader.flag_update)?
                    "\e[1;34m[\e[1;36m✔\e[1;34m]":
                    "\e[1;34m[\e[1;30m✘\e[1;34m]");
            printf("\e[1;34mCRC              %s \e[1;34m[\e[1;32m✔\e[1;34m] "
                                               "\e[1;34mCRC: \e[0;36m0x%08lX\e[0m\n",
                    (id3->extheader.flag_crc)?
                    "\e[1;34m[\e[1;36m✔\e[1;34m]":
                    "\e[1;34m[\e[1;30m✘\e[1;34m]",
                    id3->extheader.crc);
            printf("\e[1;34mRestricted       %s \e[1;34m[\e[1;31m✘\e[1;34m] \e[0;33m(Will be ignored)\e[0m\n", 
                    (id3->extheader.flag_restricted)?
                    "\e[1;34m[\e[1;36m✔\e[1;34m]":
                    "\e[1;34m[\e[1;30m✘\e[1;34m]");
        }
    }
    else
    {
        // Reset all values
        id3->extheader.size            = 0;
        id3->extheader.paddingsize     = 0;
        id3->extheader.flag_update     = false;
        id3->extheader.flag_crc        = false;
        id3->extheader.flag_restricted = false;
        id3->extheader.crc             = 0x00;
        id3->extheader.restrictions    = 0x00;
    } 

    // read all frames
    id3->framelist      = NULL;
    ID3V2_FRAME  **next = &id3->framelist;
    unsigned int offset;
    // Calculate the start of the frames
    if(id3->header.version_major == 4)
    {
        offset = id3->extheader.size;
    }
    else
    {
        if(id3->extheader.size > 0)
            offset = id3->extheader.size + 4;
        else
            offset = 0;
    }
    id3->header.realsize = offset;


    while(offset < id3->header.origsize)
    {
        ID3V2_FRAME *frame;
        // Allocate memory
        frame = (ID3V2_FRAME*) malloc(sizeof(ID3V2_FRAME));
        if(frame == NULL)
        {
            fprintf(stderr, "%s, %i: ", __FILE__, __LINE__);
            fprintf(stderr, "Fatal Error! - malloc returned NULL!\n");
            return ID3V2ERROR_FATAL;
        }

        // read frame ID
        fread(&bigendian, 4, 1, id3->file);
        frame->ID = be32toh(bigendian);
        if(frame->ID == 0x00000000) // we are in the padding area, end of ID3 reached
        {
            free(frame); // no need for this frame
            offset += 4; // 4 bytes of the padding bytes already read
            break;       // this is the end of the list, reading completed
        }

        // read size
        unsigned int encsize; 
        fread(&encsize, 4, 1, id3->file);
        if(id3->header.version_major == 4) // ID3v2.4.0 uses a different encoding
        {
            frame->size = ID3V2_DecodeSize(encsize);
        }
        else
        {
            frame->size = be32toh(encsize);
        }

        // read flags
        fread(&bigendian, 2, 1, id3->file);
        frame->flags = be16toh(bigendian);

        // read data
        frame->data = malloc(frame->size);
        if(frame->data == NULL)
        {
            fprintf(stderr, "%s, %i: ", __FILE__, __LINE__);
            fprintf(stderr, "Fatal Error! - malloc returned NULL!\n");
            return ID3V2ERROR_FATAL;
        }
        fread(frame->data, 1, frame->size, id3->file);

#ifndef DEBUG
        if(OPT_PrintHeader)
#endif
        {
            printf("\e[1;37mFrame\e[0m @ offset %6i: ", offset);
            printf("\e[1;34mID: \033[0;36m\'%c%c%c%c\'", (frame->ID >> 24) & 0xFF,
                                                         (frame->ID >> 16) & 0xFF,
                                                         (frame->ID >>  8) & 0xFF,
                                                         (frame->ID >>  0) & 0xFF);
            printf("\e[1;34m, Flags: \033[0;36m0x%04X",   frame->flags);
            printf("\e[1;34m, Size: \033[0;36m%6i\n",     frame->size);
        }

        // Put frame into list
        *next = frame;
        next  = &frame->next;

        // trace offset and real size separately, they can be different due to padding
        offset               += frame->size + 10; // 10: size of the frame header
        id3->header.realsize += frame->size + 10; // 10: size of the frame header
    }
    *next = NULL; // End of list

    // Reading done
    *id3v2 = id3;
#ifndef DEBUG
    if(OPT_PrintHeader)
#endif
    {
        printf("\e[1;33mAdjusting header size: ");
        printf("\e[1;34mOrig. size: \e[1;36m%i\e[1;34m, real size: \e[1;36m%i\e[0m\n", id3->header.origsize,
                                                                                       id3->header.realsize);
    }

    // PEDANTIC HEADER CHECK
    // check if the padding-bytes are really padding bytes or if the headers size value is wrong (may happen with bad mp3 files)
    int data;
    while(offset < id3->header.origsize)
    {
        // Get padding byte
        if((data = getc(id3->file)) == EOF)
            return ID3V2ERROR_UNEXPECTEDEOF;

        // Check if valid, if not, try to rescue the file by assuming that the headers size value is invalid
        if((char)data != 0x00)
        {
            fprintf(stderr, "Bad padding byte found at offset %i (file pos. %li) - expected at offset %i.\n", 
                    offset, ftell(id3->file) - 1, id3->header.origsize);
            fprintf(stderr, "\tInvalid byte: 0x%X\n", ((unsigned)data)&0x00FF);
            fprintf(stderr, "\tI\'ll try to fix it.  If it doesn\'t work, nothing will become worse.\n");

            // Correct the original size to the position where the padding bytes end
            id3->header.origsize = offset;

            // get a valid state in case the padding-check failed - +10 for the ID3 main header size
            fseek(id3->file, id3->header.origsize + 10, SEEK_SET);  
            break;
        }
        offset++;
    }

    // Now, the magic mp3-ID shall come
    unsigned short magicword;
    fread(&magicword, 2, 1, id3->file);
    if(magicword != 0xFAFF 
    && magicword != 0xFBFF 
    && magicword != 0xFDFF
    && magicword != 0xF3FF) // keep LE in mind
    {
        fprintf(stderr, "Magic mp3-ID not found at expected offset %i (file pos. %li)!\n", 
                offset, ftell(id3->file) - 1);
        fprintf(stderr, "\tInvalid ID: 0x%4X\n", magicword&0x0FFFF);
        fprintf(stderr, "\tAt file pos. %li\n", ftell(id3->file));
        fprintf(stderr, "\tI\'ll do nothing - nothing will become worse.\n");
    }

    // Calculate and compare CRC32 check sum if the corresponding flag is set in the header
    long endofdata;
    long startofdata;
    startofdata = 10+id3->extheader.size;   // The data to consider start after the extended header
    endofdata   = 10+id3->header.origsize;  // 10: Size of tag header
    if(id3->header.version_major == 3)      // In ID3v2.3.0 …
    {
        endofdata   -= id3->extheader.paddingsize; // … only frame are considered and …
        startofdata += 4;                          // … the ext header size variable is not count
    }

#ifndef DEBUG
    if(OPT_PrintHeader)
#endif
    {
        int error = 0;
        if(id3->extheader.flag_crc)
            error = CRC32FromFile(id3->file, startofdata, endofdata, &id3->extheader.realcrc);
        if(error == 0 )
        {
            printf("\e[1;37mCRC Check:\n");
            printf("\e[1;34mStored:     \e[1;36m0x%08lX\n", id3->extheader.crc);
            printf("\e[1;34mCalculated: %s0x%08lX\e[0m\n", 
                    (id3->extheader.crc == id3->extheader.realcrc)?"\e[1;32m":"\e[1;31m",
                    id3->extheader.realcrc);
        }
    }

    return ID3V2ERROR_NOERROR;
}

//////////////////////////////////////////////////////////////////////////////

int ID3V2_Close(ID3V2 *id3v2, const char *altpath, bool removetag)
{
    ID3V2 *id3 = id3v2; // two letters less... well, internally I use id3 so this make the code more comfortable
    int error;
    bool readonly = false;

    if(altpath != NULL)
    {
        // check if altpath is /dev/null - this means readonly-mode
        if(strncmp("/dev/null", altpath, 10) == 0)
        {
            readonly = true;
        }
        // check if altpath is the same as the orig one. If yes, do not use the altpath, use a tmpfile instead
        else if(strncmp(id3v2->path, altpath, strlen(id3v2->path)) == 0)
        {
            altpath = NULL; // use a tempfile to avoid problems like infinit copy
        }
    }

    // open second (destination) file. If altpath is given, this one is used, otherwise a temporary one
    FILE *dstfile = NULL;
    if(altpath == NULL) // open tmp file
    {
        // create temporary file
        dstfile = tmpfile();
        if(dstfile == NULL)
        {
            fprintf(stderr, "Creating tmp file failed with error \"%s\"\n", strerror(errno));
            return ID3V2ERROR_PATHERROR;
        }
    }
    else if(readonly == false)
    {
        dstfile = fopen(altpath, "w+b");
        if(dstfile == NULL)
        {
            fprintf(stderr, "Opening file \"%s\" failed with error \"%s\"\n", altpath, strerror(errno));
            return ID3V2ERROR_PATHERROR;
        }
    }
#ifdef DEBUG
    printf("\e[1;37mClosing...\n");
    printf("\e[1;34mReadonly: \e[1;36m%s\n", readonly?"true":"false");
    printf("\e[1;34mAltpath:  \e[1;36m%s\n", altpath?altpath:"\e[0;36mNULL");
#endif

    // Store header
    if(!readonly && !removetag)
    {
        fwrite(&id3->header.ID,            1, 3, dstfile); //\. 
        fwrite(&id3->header.version_major, 1, 1, dstfile); // \_ these values are always valid
        fwrite(&id3->header.version_minor, 1, 1, dstfile); // /
        fwrite(&id3->header.flags,         1, 1, dstfile); /// 

        // encode size to 7bit/byte BE
        // IMPORTANT: I use real size and my own padding bytes
        unsigned int encsize; 
        encsize = ID3V2_EncodeSize(id3->header.realsize + ID3V2PADDING);
        fwrite(&encsize, 4, 1, dstfile);

        // Write extended header if available
        if(id3->header.flags & ID3V2HEADERFLAG_EXTENDEDHEADER)
        {
            // TODO: Check size - is it a valid value? If not, get panic!
            if(id3->header.version_major == 4)
                ID3V240_WriteExtendedHeader(id3, dstfile);
            else
                ID3V230_WriteExtendedHeader(id3, dstfile);
        }
    }

    // Write frames
    ID3V2_FRAME *frame = id3->framelist;
    while(frame != NULL)
    {
        unsigned int bigendian;
        // store frame
        if(!readonly && !removetag)
        {
#ifdef DEBUG
            printf("\e[1;33mWriting… ");
#endif
            // write frame ID
            bigendian = htobe32(frame->ID);
            fwrite(&bigendian, 4, 1, dstfile);

            // write frame size
            unsigned int encsize; 
            if(id3->header.version_major == 4) // ID3v2.4.0 uses a different encoding
                encsize = ID3V2_EncodeSize(frame->size);
            else
                encsize = htobe32(frame->size);
            fwrite(&encsize, 4, 1, dstfile);

            // write frame flags
            bigendian = htobe16(frame->flags);
            fwrite(&bigendian, 2, 1, dstfile);
        
            // write frame data
            fwrite(frame->data, 1, frame->size, dstfile);
#ifdef DEBUG
            printf("\e[1;37mStored frame: ");
            printf("\e[1;34mID: \033[1;36m\'%c%c%c%c\'", (frame->ID >> 24) & 0xFF,
                                                         (frame->ID >> 16) & 0xFF,
                                                         (frame->ID >>  8) & 0xFF,
                                                         (frame->ID >>  0) & 0xFF);
            printf("\e[1;34m, Flags: \e[1;36m0x%04X",     frame->flags);
            printf("\e[1;34m, Size: \e[1;36m%6i\e[0m\n",  frame->size);
#endif
        }

        // select next frame
        ID3V2_FRAME *oldframe;
        oldframe = frame;
        frame    = frame->next;

        // free memory of old one
        free(oldframe->data);
        free(oldframe);
    }

    // Finalize tag with padding bytes and check sum
    if(!readonly && !removetag)
    {
        // Write padding bytes
        for(int i=0; i < ID3V2PADDING; i++)
            fputc(0x00, dstfile);

        // Update CRC sum when necessary
        if(id3->header.flags & ID3V2HEADERFLAG_EXTENDEDHEADER && id3->extheader.flag_crc)
        {
            // 2.3: Only frames, 2.4: frames+padding!
            if(id3->header.version_major == 4)
                error = ID3V240_UpdateCRC(id3, dstfile);
            else
                error = ID3V230_UpdateCRC(id3, dstfile);

            if(error)
                return error;
        }
    }

    if(!readonly)
    {
        // copy audio data to the new file
        // jump to the end of the ID3 infos in the orig file
        // the 10 is for the main header, old/orig size is used because the source has the orig structure
#ifdef DEBUG
        printf("\e[1;34mHeader sized: \e[0;33mOrig: \e[0;36m%i \e[0;32mNew: \e[0;36m%i\n\e[0m", 
                id3->header.origsize, id3->header.realsize);
#endif
        if(id3->rawmp3) // mp3 data starts at position 0
            fseek(id3->file,                         0, SEEK_SET);
        else            // mp3 data starts at the end of the ID3 tag (+10 for the ID3 Tag Header)
            fseek(id3->file, id3->header.origsize + 10, SEEK_SET);

        int data;
        while((data = getc(id3->file)) != EOF)
            fputc(data, dstfile);

        // if a tmp file was used, replace the source file with new content
        if(altpath == NULL)
        {
            id3->file = freopen(id3->path, "w+b", id3->file); // reopen with write permission
            if(id3->file == NULL)
            {
                fprintf(stderr, "Re-opening file \"%s\" for write access failed with error \"%s\"\n", 
                        id3->path, strerror(errno));
                return ID3V2ERROR_PATHERROR;
            }
#ifdef DEBUG
            printf("\e[1;33mCopy tmp file to destination \e[0;33m(%s)\e[1;33m…\n\e[0m", id3->path);
            printf("\t\e[1;34mPos. Temp.File: \e[1;36m%10i\n", ftell(dstfile));
            printf("\t\e[1;34mPos. Dest.File: \e[1;36m%10i\n\e[0m", ftell(id3->file));
#endif
            // go to start of both files
            error = fseek(dstfile,   0, SEEK_SET);
            if(error)
            {
                fprintf(stderr, "fseek for temp.file failed with error \"%s\"\n", strerror(errno));
                return ID3V2ERROR_WRITEERROR;
            }
            
            error = fseek(id3->file, 0, SEEK_SET);
            if(error)
            {
                fprintf(stderr, "fseek for dest.file failed with error \"%s\"\n", strerror(errno));
                return ID3V2ERROR_WRITEERROR;
            }

            // and copy from temporary file to destination file
            while((data = getc(dstfile)) != EOF)
            {
                if(fputc(data, id3->file) == EOF)
                {
                    fprintf(stderr, "Writing data from temporary file to destination failed\n");
                    return ID3V2ERROR_WRITEERROR;
                }
            }
#ifdef DEBUG
            printf("\e[1;32mdone\n\e[0m");
            printf("\t\e[1;34mPos. Temp.File: \e[1;36m%10i\n", ftell(dstfile));
            printf("\t\e[1;34mPos. Dest.File: \e[1;36m%10i\n\e[0m", ftell(id3->file));
#endif
        }
    }

    // close all files and free some memory
    if(dstfile != NULL)
        fclose(dstfile);
    fclose(id3->file);
    free(id3->path);
    free(id3);

    return ID3V2ERROR_NOERROR;
}

//////////////////////////////////////////////////////////////////////////////

static ID3V2_FRAME *GetFrameById(const ID3V2 *id3v2, unsigned int ID)
{
    ID3V2_FRAME *frame;
    frame = id3v2->framelist;
    while(frame != NULL)
    {
        if(frame->ID == ID)
            return frame;

        frame = frame->next;
    }

    return NULL;
}

//----------------------------------------------------------------------------

static void ReplaceOrAppendFrameById(ID3V2 *id3v2, ID3V2_FRAME *newframe)
{
    ID3V2_FRAME *oldframe, *prevframe;
    if(newframe == NULL)
        return;
    
    id3v2->header.realsize += (newframe->size + 10); // add the size of the new frame to the sum of all frame size incl its header

    // Check if a complete new list must be created
    if(id3v2->framelist == NULL)
    {
        newframe->next = NULL;
        id3v2->framelist = newframe;
        return;
    }

    // Check if the first frame is the one
    if(id3v2->framelist->ID == newframe->ID)
    {
        oldframe         = id3v2->framelist;
        newframe->next   = oldframe->next;
        id3v2->framelist = newframe;
    }
    else
    {
        oldframe  = id3v2->framelist->next;
        prevframe = id3v2->framelist;
        while(oldframe != NULL)
        {
            if(oldframe->ID == newframe->ID)
            {
                newframe->next  = oldframe->next;
                prevframe->next = newframe; 
                break;
            }

            prevframe = oldframe;
            oldframe  = oldframe->next;
        }
    }

    if(oldframe != NULL)    // delete old one
    {
        id3v2->header.realsize -= (oldframe->size + 10); // sub the size of the old frame incl its header
        if(oldframe->data != NULL)
            free(oldframe->data);
        free(oldframe);
    }
    else                    // no old one? so the new one must be appended
    {
        newframe->next  = NULL;
        prevframe->next = newframe;
    }

    return;
}

//////////////////////////////////////////////////////////////////////////////

int ID3V2_RemoveAllFrames(ID3V2 *id3v2)
{
    ID3V2_FRAME *nextframe, *oldframe;
    nextframe = id3v2->framelist;
    while(nextframe != NULL)
    {
        oldframe  = nextframe;
        nextframe = nextframe->next;

        id3v2->header.realsize -= (oldframe->size + 10); // sub size of data and header of the frame
        if(oldframe->data != NULL)
            free(oldframe->data);
        free(oldframe);
    }
    id3v2->framelist = NULL;
    return 0;
}

//////////////////////////////////////////////////////////////////////////////

int ID3V2_GetFrame(const ID3V2 *id3v2, unsigned int ID, size_t *size, void **data)
{
#ifdef DEBUG
        printf("\e[1;37mGet frame: ");
        printf("\e[1;34mID: \033[1;36m\'%c%c%c%c\'\n", (ID >> 24) & 0xFF,
                                                       (ID >> 16) & 0xFF,
                                                       (ID >>  8) & 0xFF,
                                                       (ID >>  0) & 0xFF);
#endif
    ID3V2_FRAME *frame;
    frame = GetFrameById(id3v2, ID);
    if(frame == NULL)
        return ID3V2ERROR_FRAMENOTFOUND;

    if(size != NULL)
        *size = frame->size;

    if(data != NULL)
    {
        *data = malloc(frame->size);
        if(*data == NULL)
        {
            fprintf(stderr, "%s, %i: ", __FILE__, __LINE__);
            fprintf(stderr, "Fatal Error! - malloc returned NULL!\n");
            return ID3V2ERROR_FATAL;
        }
        memcpy(*data, frame->data, frame->size);
    }

    return ID3V2ERROR_NOERROR;
}

//////////////////////////////////////////////////////////////////////////////

int ID3V2_SetFrame(ID3V2 *id3v2, unsigned int ID, size_t size, const void *data)
{
#ifdef DEBUG
        printf("\e[1;37mSet frame: ");
        printf("\e[1;34mID: \033[1;36m\'%c%c%c%c\'\n", (ID >> 24) & 0xFF,
                                                       (ID >> 16) & 0xFF,
                                                       (ID >>  8) & 0xFF,
                                                       (ID >>  0) & 0xFF);
#endif
    // Allocate memory
    ID3V2_FRAME *frame;
    frame = malloc(sizeof(ID3V2_FRAME));
    if(frame == NULL)
    {
        fprintf(stderr, "%s, %i: ", __FILE__, __LINE__);
        fprintf(stderr, "Fatal Error! - malloc returned NULL!\n");
        return ID3V2ERROR_FATAL;
    }

    frame->data = malloc(size);
    if(frame->data == NULL)
    {
        fprintf(stderr, "%s, %i: ", __FILE__, __LINE__);
        fprintf(stderr, "Fatal Error! - malloc returned NULL!\n");
        return ID3V2ERROR_FATAL;
    }

    // Set values
    frame->ID   = ID;
    frame->flags= 0x0000;
    frame->size = size;
    memcpy(frame->data, data, size);

    // Replace (or append)
    ReplaceOrAppendFrameById(id3v2, frame);
    return ID3V2ERROR_NOERROR;
}

// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

